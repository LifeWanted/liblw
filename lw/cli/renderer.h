#pragma once

#include <cstdint>

#include "lw/cli/color.h"
#include "lw/cli/image.h"

namespace lw::cli {

/**
 * Buffered drawing device.
 */
class Renderer {
public:
  Renderer(UIVector2d dimensions):
    _buffer{dimensions},
    _render_dimensions{dimensions}
  {}

  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = default;
  Renderer& operator=(Renderer&&) = default;

  const Image& image() const { return _buffer; }

  void move_to(UIVector2d position) {
    _position = position;
  }

  void move_to_start() {
    _position = {0, 0};
  }

  void move_to_end() {
    _position = {_buffer.width() - 1, _buffer.height() - 1};
  }

  void set_background(const Color& color);

  void clear_draw_background() { _background.a = 0; }
  void set_draw_background(const Color& color) { _background = color; }
  void set_draw_color(const Color& color) { _foreground = color; }

  void clear(const Color& background) {
    _background = background;
    clear();
  }

  void clear();

  void draw(const Pixel& pixel) {
    _blend_pixel(pixel);
    _advance_position();
  }

  void draw(char c) {
    draw(static_cast<char32_t>(c));
  }

  void draw(char32_t glyph);

  void draw(const Image& image, Box crop);
  void draw(const Image& image) {
    draw(image, {{0, 0}, image.dimensions()});
  }

  void blend_foreground();

  void constrain_dimensions(UIVector2d dimensions) {
    _render_dimensions = {
      std::min(_buffer.width(), dimensions.x),
      std::min(_buffer.height(), dimensions.y)
    };
  }
  const UIVector2d& dimensions() const { return _render_dimensions; }

private:
  bool _position_in_constraint() const;
  void _advance_position();
  void _advance_column();
  void _advance_line();
  void _blend_pixel(const Pixel& p);
  Color _alphablend(Color a, Color b);
  std::uint8_t _blend_channel(
    std::uint8_t dest,
    std::uint8_t src,
    std::uint8_t dest_alpha,
    std::uint8_t src_alpha
  );

  Image _buffer;
  UIVector2d _render_dimensions;
  Color _background = Color::transparent();
  Color _foreground;
  UIVector2d _position = {0, 0};
};

}
