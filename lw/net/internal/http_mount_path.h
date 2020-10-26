#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace lw::net::internal {

class PathMatcher {
public:
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

  std::optional<std::unordered_map<std::string_view, std::string_view>>
  match(std::string_view url_path) const;

private:
  std::vector<std::unique_ptr<PathMatcher>> _matchers;
};

}
