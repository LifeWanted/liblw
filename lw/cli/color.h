#pragma once

#include <cstdint>

namespace lw::cli {

struct rgb_t { explicit rgb_t() = default; };
struct bgr_t { explicit bgr_t() = default; };
struct argb_t { explicit argb_t() = default; };
struct rgba_t { explicit rgba_t() = default; };

constexpr rgb_t rgb;
constexpr bgr_t bgr;
constexpr argb_t argb;
constexpr rgba_t rgba;

struct Color {
  Color(): Color(0x000000) {}
  Color(std::uint32_t rgb):
    r{static_cast<std::uint8_t>((rgb & 0x00ff0000) >> 16)},
    g{static_cast<std::uint8_t>((rgb & 0x0000ff00) >>  8)},
    b{static_cast<std::uint8_t>((rgb & 0x000000ff) >>  0)}
  {}
  Color(std::uint8_t r, std::uint8_t g, std::uint8_t b): r{r}, g{g}, b{b} {}
  Color(std::uint8_t a, std::uint8_t r, std::uint8_t g, std::uint8_t b):
    r{r}, g{g}, b{b}, a{a}
  {}

  Color(rgb_t, std::uint8_t r, std::uint8_t g, std::uint8_t b):
    r{r}, g{g}, b{b}
  {}
  Color(bgr_t, std::uint8_t b, std::uint8_t g, std::uint8_t r):
    r{r}, g{g}, b{b}
  {}
  Color(argb_t, std::uint8_t a, std::uint8_t r, std::uint8_t g, std::uint8_t b):
    r{r}, g{g}, b{b}, a{a}
  {}
  Color(rgba_t, std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a):
    r{r}, g{g}, b{b}, a{a}
  {}

  Color(rgb_t, std::uint32_t rgb):
    r{static_cast<std::uint8_t>((rgb & 0x00ff0000) >> 16)},
    g{static_cast<std::uint8_t>((rgb & 0x0000ff00) >>  8)},
    b{static_cast<std::uint8_t>((rgb & 0x000000ff) >>  0)}
  {}
  Color(bgr_t, std::uint32_t bgr):
    r{static_cast<std::uint8_t>((bgr & 0x000000ff) >>  0)},
    g{static_cast<std::uint8_t>((bgr & 0x0000ff00) >>  8)},
    b{static_cast<std::uint8_t>((bgr & 0x00ff0000) >> 16)}
  {}
  Color(argb_t, std::uint32_t argb):
    r{static_cast<std::uint8_t>((argb & 0x00ff0000) >> 16)},
    g{static_cast<std::uint8_t>((argb & 0x0000ff00) >>  8)},
    b{static_cast<std::uint8_t>((argb & 0x000000ff) >>  0)},
    a{static_cast<std::uint8_t>((argb & 0xff000000) >> 24)}
  {}
  Color(rgba_t, std::uint32_t rgba):
    r{static_cast<std::uint8_t>((rgba & 0xff000000) >> 24)},
    g{static_cast<std::uint8_t>((rgba & 0x00ff0000) >> 16)},
    b{static_cast<std::uint8_t>((rgba & 0x0000ff00) >>  8)},
    a{static_cast<std::uint8_t>((rgba & 0x000000ff) >>  0)}
  {}

  static const Color& transparent() {
    static const Color c{0, 0, 0, 0};
    return c;
  }

  static const Color& black() {
    static const Color c{0, 0, 0};
    return c;
  }

  static const Color& white() {
    static const Color c{255, 255, 255};
    return c;
  }

  static const Color& red() {
    static const Color c{255, 0, 0};
    return c;
  }

  static const Color& green() {
    static const Color c{0, 255, 0};
    return c;
  }

  static const Color& blue() {
    static const Color c{0, 0, 255};
    return c;
  }

  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
  std::uint8_t a = 255;

  bool operator==(const Color& other) const {
    return (
      reinterpret_cast<const std::uint32_t&>(*this) ==
      reinterpret_cast<const std::uint32_t&>(other)
    );
  }
  bool operator!=(const Color& other) const {
    return !(*this == other);
  }
};

class Palette {
public:
  virtual const Color& transparent() const { return Color::transparent(); }
  virtual const Color& black() const { return Color::black(); }
  virtual const Color& white() const { return Color::white(); }
  virtual const Color& red() const { return Color::red(); }
  virtual const Color& green() const { return Color::green(); }
  virtual const Color& blue() const { return Color::blue(); }

  virtual const Color& background() const { return transparent(); }
  virtual const Color& foreground() const { return white(); }
};

}
