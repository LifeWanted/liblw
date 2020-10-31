#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <vector>

#include "lw/http/http_handler.h"

namespace lw::http::internal {

class EndpointTrie;

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

  std::optional<std::unordered_map<std::string_view, std::string_view>>
  match(std::string_view url_path) const;

private:
  friend class EndpointTrie;

  std::vector<std::unique_ptr<PathMatcher>> _matchers;
};


class EndpointTrie {
public:
  struct MatchResult {
    std::unordered_map<std::string_view, std::string_view> parameters;
    const BaseHttpHandlerFactory& endpoint;
  };

  template <typename HttpHandlerFactory>
  void insert(MountPath&& mount_path, HttpHandlerFactory&& endpoint) {
    TrieNode* node = _build_path(std::move(mount_path), endpoint);
    node->endpoint = std::make_unique<HttpHandlerFactory>(
      std::forward<HttpHandlerFactory>(endpoint)
    );
  }

  std::optional<MatchResult> match(std::string_view url_path) const;
private:
  struct TrieNode {
    std::unique_ptr<BaseHttpHandlerFactory> endpoint;
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    std::optional<
      std::pair<std::unique_ptr<PathMatcher>, std::unique_ptr<TrieNode>>
    > wildcard;
  };

  TrieNode* _build_path(
    MountPath&& mount_path,
    const BaseHttpHandlerFactory& endpoint
  );
  TrieNode* _build_literal_path(TrieNode* root, std::string_view part);

  TrieNode _root;
};

}