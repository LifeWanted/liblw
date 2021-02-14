#pragma once

#include <cstdint>

namespace lw::cli {

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
