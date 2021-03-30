#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lw/http/headers.h"
#include "lw/http/http_handler.h"

namespace lw::http::internal {

class BaseEndpointTrie;

class PathMatcher {
public:
  virtual ~PathMatcher() = default;

  virtual bool is_literal() const = 0;
  virtual std::string_view name() const = 0;
  virtual std::string_view chunk() const = 0;
  virtual std::optional<std::string_view> match(
    std::string_view url_part
  ) const = 0;
};

class MountPath {
public:
  static MountPath parse_endpoint(std::string_view endpoint);

  MountPath(std::vector<std::unique_ptr<PathMatcher>> matchers):
    _matchers{std::move(matchers)}
  {}
  MountPath(const MountPath&) = default;
  MountPath(MountPath&&) = default;
  MountPath& operator=(const MountPath&) = default;
  MountPath& operator=(MountPath&&) = default;

  std::optional<http::HeadersView> match(std::string_view url_path) const;

private:
  friend class BaseEndpointTrie;

  std::vector<std::unique_ptr<PathMatcher>> _matchers;
};

class BaseEndpointTrie {
public:
  struct BaseTrieNode {
    std::unordered_map<char, std::unique_ptr<BaseTrieNode>> children;
    std::optional<
      std::pair<std::unique_ptr<PathMatcher>, std::unique_ptr<BaseTrieNode>>
    > wildcard;
  };

  struct BaseMatchResult {
    http::HeadersView parameters;
    const BaseTrieNode* node;
  };

  virtual std::unique_ptr<BaseTrieNode> make_node() = 0;
  BaseTrieNode* build_path(MountPath&& mount_path, std::string_view route);
  BaseMatchResult walk_path(std::string_view path) const;

private:
  BaseTrieNode* _build_literal_path(BaseTrieNode* root, std::string_view part);

  BaseTrieNode _root;
};

template <typename Endpoint>
class EndpointTrie: private BaseEndpointTrie {
public:
  struct MatchResult {
    http::HeadersView parameters;
    const Endpoint& endpoint;
  };

  void insert(MountPath&& mount_path, const Endpoint& endpoint) {
    TrieNode* node = static_cast<TrieNode*>(
      build_path(std::move(mount_path), endpoint.route())
    );
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
    node->endpoint = &endpoint;
  }

  std::optional<MatchResult> match(std::string_view url_path) const {
    BaseMatchResult result = walk_path(url_path);
    if (result.node && static_cast<const TrieNode*>(result.node)->endpoint) {
      return MatchResult{
        .parameters = std::move(result.parameters),
        .endpoint = *static_cast<const TrieNode*>(result.node)->endpoint
      };
    }
    return std::nullopt;
  }
private:
  struct TrieNode: public BaseTrieNode {
    const Endpoint* endpoint = nullptr;
  };

  std::unique_ptr<BaseTrieNode> make_node() override {
    return std::make_unique<TrieNode>();
  }
};

}
