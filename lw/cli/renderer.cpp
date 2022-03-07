#include "lw/cli/renderer.h"

namespace lw::cli {

void Renderer::set_background(const Color& color) {
  for (Pixel& p : _buffer.pixels()) {
    p.bg = color;
  }
}

void Renderer::clear() {
  Pixel blanked{_background, Color::transparent(), U' '};
  for (Pixel& p : _buffer.pixels()) {
    p = blanked;
  }
  _position = {0, 0};
}

void Renderer::draw(char32_t glyph) {
  if (_position_in_constraint()) {
    Pixel& p = _buffer.pixel(_position);
    p.glyph = glyph;
    p.fg = _foreground;
    if (_background.a) p.bg = _background;
  }
  _advance_position();
}

void Renderer::draw(const Image& image, Box crop) {
  std::size_t starting_x = _position.x;
  for (std::size_t y = crop.position.y; y < crop.size.y; ++y) {
    for (std::size_t x = crop.position.x; x < crop.size.x; ++x) {
      if (_position_in_constraint()) _blend_pixel(image.pixel(x, y));
      _advance_column();
    }
    _advance_line();
    _position.x = starting_x;
  }
}

void Renderer::blend_foreground() {
  for (Pixel& p : _buffer.pixels()) {
    p.fg = _alphablend(p.bg, p.fg);
    p.fg.a = 255;
  }
}

bool Renderer::_position_in_constraint() const {
  return (
    _position.x < _render_dimensions.x &&
    _position.y < _render_dimensions.y
  );
}

void Renderer::_advance_position() {
  ++_position.x;
  if (_position.x >= _buffer.width()) {
    _position.x = 0;
    _advance_line();
  }
}

void Renderer::_advance_column() {
  ++_position.x;
  if (_position.x >= _buffer.width()) {
    _position.x = 0;
  }
}

void Renderer::_advance_line() {
  ++_position.y;
  if (_position.y >= _buffer.height()) _position.y = 0;
}

void Renderer::_blend_pixel(const Pixel& p) {
  Pixel& b = _buffer.pixel(_position);
  b.bg = _alphablend(b.bg, p.bg);
  b.fg = _alphablend(b.fg, p.fg);
  if (p.fg.a) b.glyph = p.glyph;
}

Color Renderer::_alphablend(Color a, Color b) {
  a.r = _blend_channel(a.r, b.r, a.a, b.a);
  a.g = _blend_channel(a.g, b.g, a.a, b.a);
  a.b = _blend_channel(a.b, b.b, a.a, b.a);

  double b_alpha = b.a / 255.0;
  a.a = (b_alpha + ((a.a / 255.0) * (1.0 - b_alpha) )) * 255;
  return a;
}

std::uint8_t Renderer::_blend_channel(
  std::uint8_t dest,
  std::uint8_t src,
  std::uint8_t dest_alpha,
  std::uint8_t src_alpha
) {
  if (src_alpha == 255 || dest_alpha == 0) return src;
  if (src_alpha == 0) return dest;

  double alpha = src_alpha / 255.0;
  double out_alpha = alpha + ((dest_alpha / 255.0) * (1.0 - alpha));
  return static_cast<std::uint8_t>(
    (
      (static_cast<double>(src) * alpha) +
      (static_cast<double>(dest) * (1.0 - alpha))
    ) / out_alpha
  );
}

}
