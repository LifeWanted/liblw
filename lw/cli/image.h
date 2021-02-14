#pragma once

#include <vector>

#include "lw/cli/color.h"

namespace lw::cli {

struct IVector2d {
  std::int64_t x;
  std::int64_t y;
};

struct UIVector2d {
  std::uint64_t x;
  std::uint64_t y;
};

struct Box {
  UIVector2d position;
  UIVector2d size;
};

struct Pixel {
  Color bg = {0, 0, 0, 0};
  Color fg = {0, 0, 0, 0};
  char32_t glyph = U' ';
};

class Image {
public:
  explicit Image(UIVector2d size):
    _pixels{size.x * size.y},
    _size{std::move(size)}
  {}
  explicit Image(const Image&) = default;
  Image& operator=(const Image&) = delete;
  Image(Image&&) = default;
  Image& operator=(Image&&) = default;

  /**
   * Copy the image data from the other image to this one.
   *
   * This is a potentially expensive operation and thus is made an explicit
   * method call instead of allowing `operator=`.
   */
  void copy(const Image& other) {
    _pixels = other._pixels;
    _size = other._size;
  }

  std::size_t width() const { return _size.x; }
  std::size_t height() const { return _size.y; }
  const UIVector2d& dimensions() const { return _size; }

  Pixel& pixel(const UIVector2d& pos) {
    return _pixels[pos.x + (pos.y * width())];
  }
  const Pixel& pixel(const UIVector2d& pos) const {
    return _pixels[pos.x + (pos.y * width())];
  }
  Pixel& pixel(std::size_t x, std::size_t y) {
    return _pixels[x + (y * width())];
  }
  const Pixel& pixel(std::size_t x, std::size_t y) const {
    return _pixels[x + (y * width())];
  }
  std::vector<Pixel>& pixels() { return _pixels; }
  const std::vector<Pixel>& pixels() const { return _pixels; }

private:
  std::vector<Pixel> _pixels;
  UIVector2d _size;
};

}
