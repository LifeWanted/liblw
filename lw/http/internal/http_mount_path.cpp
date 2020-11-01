#include "lw/http/internal/http_mount_path.h"

#include <cctype>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "lw/base/strings.h"
#include "lw/err/canonical.h"
#include "lw/http/headers.h"
#include "lw/http/http_handler.h"

namespace lw::http::internal {
namespace {

const char SEP = '/';

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//                                                                            //
//                  # #   ##  #####  ### #  # #### ###   ##                   //
//                 # # # #  #   #   #    #  # #    #  # #                     //
//                 # # # ####   #   #    #### ###  ###   ##                   //
//                 #   # #  #   #   #    #  # #    #  #    #                  //
//                 #   # #  #   #    ### #  # #### #  #  ##                   //
//                                                                            //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

class LiteralPathMatcher: public PathMatcher {
public:
  explicit LiteralPathMatcher(std::string_view chunk): _chunk{chunk} {}
  LiteralPathMatcher(const LiteralPathMatcher&) = default;
  LiteralPathMatcher(LiteralPathMatcher&&) = default;
  LiteralPathMatcher& operator=(const LiteralPathMatcher&) = default;
  LiteralPathMatcher& operator=(LiteralPathMatcher&&) = default;

  bool is_literal() const override { return true; }
  std::string_view name() const override { return _chunk; }
  std::string_view chunk() const override { return _chunk; }
  std::optional<std::string_view> match(
    std::string_view url_part
  ) const override {
    CaseInsensitiveEqual eql;
    if (eql(_chunk, url_part)) return url_part;
    return std::nullopt;
  }

private:
  std::string _chunk;
};

class ParameterPathMatcher: public PathMatcher {
public:
  ParameterPathMatcher(
    std::string_view chunk,
    std::string_view name,
    std::string_view extension
  ):
    _chunk{chunk}
  {
    _extension = std::string_view{
      _chunk.data() + (_chunk.size() - extension.size()),
      extension.size()
    };

    _name = std::string_view{
      _chunk.data() + (name.begin() - chunk.begin()),
      _extension.begin()
    };
  }

  bool is_literal() const override { return false; }
  std::string_view name() const override { return _name; }
  std::string_view chunk() const override { return _chunk; }
  std::optional<std::string_view> match(
    std::string_view url_part
  ) const override {
    if (_extension.empty()) return url_part;

    if (url_part.size() <= _extension.size()) return std::nullopt;
    CaseInsensitiveEqual eql;
    std::string_view part_end{
      url_part.end() - _extension.size(),
      url_part.end()
    };
    if (!eql(_extension, part_end)) return std::nullopt;
    return std::string_view{url_part.begin(), part_end.begin()};
  }

private:
  std::string _chunk;
  std::string_view _name;
  std::string_view _extension;
};

class ValidatedParameterPathMatcher: public ParameterPathMatcher {
public:
  typedef bool(*validator_function)(std::string_view);

  static validator_function validator_for(std::string_view type) {
    if (type == "int") return &_int_validator;
    if (type == "uint") return &_uint_validator;
    throw InvalidArgument() << "Unknown parameter validation type: " << type;
  }

  ValidatedParameterPathMatcher(
    std::string_view chunk,
    std::string_view name,
    std::string_view extension,
    validator_function validator
  ):
    ParameterPathMatcher(chunk, name, extension),
    _validator{validator}
  {}

  std::optional<std::string_view> match(
    std::string_view url_part
  ) const override {
    std::optional<std::string_view> sub_part =
      ParameterPathMatcher::match(url_part);
    if (sub_part && _validator(*sub_part)) return sub_part;
    return std::nullopt;
  }

private:
  static bool _int_validator(std::string_view url_part) {
    if (url_part[0] == '-') url_part.remove_prefix(1);
    return _uint_validator(url_part);
  }

  static bool _uint_validator(std::string_view url_part) {
    for (const char& c : url_part) {
      if (!std::isdigit(c)) return false;
    }
    return true;
  }

  validator_function _validator;
};

class RegexPathMatcher: public PathMatcher {
public:
  explicit RegexPathMatcher(std::size_t index, std::string_view chunk):
    _name{std::to_string(index)},
    _chunk{chunk}
  {
    // Prefix for regex chunks: `:[re]` (5 chars).
    _regex = std::regex{
      _chunk.begin() + 5,
      _chunk.end(),
      std::regex::ECMAScript | std::regex::icase | std::regex::optimize
    };
  }

  bool is_literal() const override { return false; }
  std::string_view name() const override { return _name; }
  std::string_view chunk() const override { return _chunk; }
  std::optional<std::string_view> match(
    std::string_view url_part
  ) const override {
    std::cmatch results;
    if (!std::regex_match(url_part.begin(), url_part.end(), results, _regex)) {
      return std::nullopt;
    }
    if (_regex.mark_count() == 0) return url_part;
    return std::string_view{results[1].first, results[1].second};
  }

private:
  std::string _name;
  std::string _chunk;
  std::regex _regex;
};

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//                                                                            //
//                      ####   ##  ###   ##  #### ###                         //
//                      #   # #  # #  # #    #    #  #                        //
//                      ####  #### ###   ##  ###  ###                         //
//                      #     #  # #  #    # #    #  #                        //
//                      #     #  # #  #  ##  #### #  #                        //
//                                                                            //
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

std::vector<std::unique_ptr<PathMatcher>> parse_into_matchers(
  std::string_view endpoint
) {
  // Test cases:
  //  - /foo/bar          -> "/foo/bar"
  //  - /foo/:bar/baz     -> "/foo/any/baz", "/foo/other/baz"
  //  - /foo/:[int]baz    -> "/foo/1", "/foo/2"
  //  - /foo/:[re].*\\..* -> "/foo/some.file", "/foo/other.xml"
  //  - /foo/:bar.json    -> "/foo/any.json", "/foo/other.json"
  const char PARAM      = ':';
  const char TYPE_START = '[';
  const char TYPE_END   = ']';
  const char FILE_EXT   = '.';
  const char ESCAPE     = '\\';
  enum State {
    START,
    LITERAL,
    PARAMETER_START,
    PARAMETER_TYPE,
    PARAMETER_NAME,
    PARAMETER_EXTENSION,
    PARAMETER_END,
    REGEX
  };
  State state = START;
  std::string_view chunk;
  std::string_view name;
  std::string_view extension;
  std::string_view type;
  const char* chunk_start = endpoint.begin();
  const char* name_start = nullptr;
  const char* extension_start = nullptr;
  const char* type_start = nullptr;
  bool escaping = false;
  std::size_t unnamed_counter = 0;
  std::vector<std::unique_ptr<PathMatcher>> matchers;

  for (std::size_t i = 0; i <= endpoint.size(); ++i) {
    const char c = (i < endpoint.size()) ? endpoint.at(i) : '\0';
    switch (state) {
      case PARAMETER_END: {
        chunk = std::string_view{chunk_start, endpoint.begin() + i - 1};
        name_start = nullptr;
        if (type.empty()) {
          matchers.push_back(
            std::make_unique<ParameterPathMatcher>(chunk, name, extension)
          );
        } else {
          matchers.push_back(
            std::make_unique<ValidatedParameterPathMatcher>(
              chunk,
              name,
              extension,
              ValidatedParameterPathMatcher::validator_for(type)
            )
          );
        }
        state = START;
      } // Fallthrough.
      case START: {
        if (i == endpoint.size() || c == SEP) continue;
        chunk_start = endpoint.begin() + i;
        state = (c == PARAM) ? PARAMETER_START : LITERAL;
        break;
      }
      case LITERAL: {
        if (i == endpoint.size() || c == SEP) {
          chunk = std::string_view{chunk_start, endpoint.begin() + i};
          matchers.push_back(std::make_unique<LiteralPathMatcher>(chunk));
          state = START;
        }
        break;
      }
      case PARAMETER_START: {
        if (i == endpoint.size()) {
          throw InvalidArgument()
            << "Unexpected end of sequence after parameter start in endpoint "
            << endpoint;
        }
        if (c == TYPE_START) {
          type_start = endpoint.begin() + i + 1;
          state = PARAMETER_TYPE;
          continue;
        }
        if (!std::isalnum(c)) {
          throw InvalidArgument()
            << "Unexpected character \'" << c << "\' at character "
            << i << " in endpoint " << endpoint
            << ". Expected either start of parameter type ([) or alpha numeric "
               "parameter name.";
        }
        name_start = endpoint.begin() + i;
        state = PARAMETER_NAME;
        break;
      }
      case PARAMETER_TYPE: {
        if (i == endpoint.size()) {
          throw InvalidArgument()
            << "Unexpected end of sequence in parameter type in endpoint "
            << endpoint;
        }
        if (c == TYPE_END) {
          type = std::string_view{type_start, endpoint.begin() + i};
          if (type.empty()) {
            throw InvalidArgument()
              << "Unexpected end of parameter type at character "
              << i << " in endpoint " << endpoint;
          }
          if (type == "re") {
            ++unnamed_counter;
            state = REGEX;
          } else {
            state = PARAMETER_NAME;
          }
          continue;
        }
        break;
      }
      case PARAMETER_NAME: {
        if (name_start == nullptr) name_start = endpoint.begin() + i;
        if (i == endpoint.size() || c == FILE_EXT || c == SEP)  {
          name = std::string_view{name_start, endpoint.begin() + i};
          if (name.empty()) {
            throw InvalidArgument()
              << "Unexpected end of parameter name at character "
              << i << " in endpoint " << endpoint;
          }
          if (c == FILE_EXT) {
            extension_start = endpoint.begin() + i;
            state = PARAMETER_EXTENSION;
          } else {
            extension = {};
            state = PARAMETER_END;
          }
          continue;
        }
        if (!std::isalnum(c)) {
          name_start = nullptr;
          throw InvalidArgument()
            << "Invalid character \'" << c << "\' at character "
            << i << " in endpoint " << endpoint
            << ". Parameter names must be alphanumeric.";
        }
        break;
      }
      case PARAMETER_EXTENSION: {
        if (i == endpoint.size() || c == SEP) {
          extension = std::string_view(extension_start, endpoint.begin() + i);
          if (extension.size() == 1) { // Just "."
            throw InvalidArgument()
              << "Unexpected end of parameter file extension at character "
              << i << " in endpoint " << endpoint;
          }
          state = PARAMETER_END;
          continue;
        }
        if (c != FILE_EXT && !std::isalnum(c)) {
          throw InvalidArgument()
            << "Invalid character \'" << c << "\' at character "
            << i << " in endpoint " << endpoint
            << ". File extensions must be alphanumeric.";
        }
        break;
      }
      case REGEX: {
        if (escaping) {
          escaping = false;
          continue;
        }
        if (c == ESCAPE) escaping = true;
        else if (i == endpoint.size() || c == SEP) {
          chunk = std::string_view{chunk_start, endpoint.begin() + i};
          matchers.push_back(
            std::make_unique<RegexPathMatcher>(unnamed_counter, chunk)
          );
          state = START;
        }
        break;
      }
    }
  }

  // We might have reached the end while parsing a parameter.
  if (state == PARAMETER_END) {
    chunk = std::string_view{chunk_start, endpoint.end()};
    name_start = nullptr;
    if (type.empty()) {
      matchers.push_back(
        std::make_unique<ParameterPathMatcher>(chunk, name, extension)
      );
    } else {
      matchers.push_back(
        std::make_unique<ValidatedParameterPathMatcher>(
          chunk,
          name,
          extension,
          ValidatedParameterPathMatcher::validator_for(type)
        )
      );
    }
  }

  return matchers;
}

}

// -------------------------------------------------------------------------- //

MountPath MountPath::parse_endpoint(std::string_view endpoint) {
  return MountPath{parse_into_matchers(endpoint)};
}

std::optional<HeadersView> MountPath::match(std::string_view url_path) const {
  std::string_view url_part;
  const char* part_start = url_path.begin();
  HeadersView parameters;
  auto match_itr = _matchers.begin();
  std::size_t i = 0;
  for (; i <= url_path.size() && match_itr != _matchers.end(); ++i) {
    if (i != url_path.size() && url_path[i] != SEP) {
      if (part_start == nullptr) part_start = url_path.begin() + i;
      continue;
    }

    url_part = std::string_view(part_start, url_path.begin() + i);
    part_start = nullptr;
    if (url_part.empty()) continue;

    auto value = (*match_itr)->match(url_part);
    if (!value) return std::nullopt;
    if (!(*match_itr)->is_literal()) {
      parameters[(*match_itr)->name()] = *value;
    }
    ++match_itr;
  }

  if (i < url_path.size() || match_itr != _matchers.end()) {
    return std::nullopt;
  }

  return parameters;
}

std::optional<EndpointTrie::MatchResult> EndpointTrie::match(
  std::string_view url_path
) const {
  HeadersView parameters;
  const TrieNode* node = &_root;
  std::vector<std::pair<const TrieNode*, std::size_t>> wildcard_stack;
  std::size_t i = 0;
  for (; i < url_path.size(); ++i) {
    if (node->wildcard) wildcard_stack.push_back(std::make_pair(node, i));
    if (node->children.contains(url_path[i])) {
      node = node->children.at(url_path[i]).get();
      continue;
    }
    while (!wildcard_stack.empty()) {
      auto [wild_node, wild_i] = wildcard_stack.back();
      wildcard_stack.pop_back();

      // Pull out the URL part this matcher will look against.
      std::size_t sep_pos = url_path.find(SEP, wild_i + 1);
      if (sep_pos == std::string_view::npos) sep_pos = url_path.size();
      std::string_view url_part{&url_path[wild_i], &url_path[sep_pos]};

      const auto& [matcher, matched_node] = *wild_node->wildcard;
      std::optional<std::string_view> wild_results = matcher->match(url_part);
      if (wild_results) {
        // TODO(alaina): Remove parameters from false path matches.
        //
        // A: /foo/:param2/baz
        // B: /:param1/:param3/other
        //
        // Request for `/foo/something/other` will match `:param2 = something`
        // then backtrack and match `:param1 = foo` and `:param3 = something`
        // before executing handler B.
        //
        // This exposes to handler B a small amount of information about handler
        // A. If B had a colliding parameter name, its would override the
        // previous value set in A.

        parameters[matcher->name()] = *wild_results;
        i = wild_i + wild_results->size() - 1; // Account for increment.
        node = matched_node.get();
        goto for_end;
      }
    }

    // No matches.
    return std::nullopt;

    for_end:;
  }

  if (node && node->endpoint) {
    return MatchResult{
      .parameters = std::move(parameters),
      .endpoint = *node->endpoint
    };
  }
  return std::nullopt;
}

EndpointTrie::TrieNode* EndpointTrie::_build_path(
  MountPath&& mount_path,
  const BaseHttpHandlerFactory& endpoint
) {
  TrieNode* node = &_root;

  for (auto& matcher : mount_path._matchers) {
    node = _build_literal_path(node, "/");
    if (matcher->is_literal()) {
      node = _build_literal_path(node, matcher->chunk());
      continue;
    }
    if (!node->wildcard) {
      node->wildcard = std::make_pair(
        std::move(matcher),
        std::make_unique<TrieNode>()
      );
      node->wildcard->second->endpoint = nullptr;
    } else if (node->wildcard->first->chunk() != matcher->chunk()) {
      throw AlreadyExists()
        << "Path parameter " << matcher->chunk() << " for route "
        << endpoint.route() << " collides with existing path parameter "
        << node->wildcard->first->chunk();
    }
    node = node->wildcard->second.get();
  }

  if (node->endpoint) {
    if (node->endpoint->route() == endpoint.route()) {
      throw AlreadyExists()
        << "A handler already exists for the route " << endpoint.route();
    } else {
      throw AlreadyExists()
        << "Handler route " << endpoint.route()
        << " collides with existing route " << node->endpoint->route();
    }
  }

  return node;
}

EndpointTrie::TrieNode* EndpointTrie::_build_literal_path(
  TrieNode* root,
  std::string_view part
) {
  for (char c : part) {
    if (!root->children.contains(c)) {
      root->children.emplace(c, std::make_unique<TrieNode>());
    }
    root = root->children[c].get();
  }
  return root;
}

}
