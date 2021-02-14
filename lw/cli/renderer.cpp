#include <algorithm>
#include <cctype>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include <variant>
#include <vector>

#include "lw/base/host_info.h"
#include "lw/cli/color.h"
#include "lw/cli/image.h"
#include "lw/cli/input.h"
#include "lw/cli/terminal.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"

namespace lw::cli {

/**
 * Buffered drawing device.
 */
class Renderer {
public:
  Renderer(UIVector2d dimensions): _buffer{dimensions} {}

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

  void set_background(const Color& color) {
    for (Pixel& p : _buffer.pixels()) {
      p.bg = color;
    }
  }

  void clear_draw_background() { _background.a = 0; }
  void set_draw_background(const Color& color) { _background = color; }
  void set_draw_color(const Color& color) { _foreground = color; }

  void clear(const Color& background) {
    _background = background;
    clear();
  }

  void clear() {
    Pixel blanked{_background, Color::transparent(), U' '};
    for (Pixel& p : _buffer.pixels()) {
      p = blanked;
    }
    _position = {0, 0};
  }

  void draw(const Pixel& pixel) {
    _blend_pixel(pixel);
    _advance_position();
  }

  void draw(char c) {
    draw(static_cast<char32_t>(c));
  }

  void draw(char32_t glyph) {
    Pixel& p = _buffer.pixel(_position);
    p.glyph = glyph;
    p.fg = _foreground;
    if (_background.a) p.bg = _background;
    _advance_position();
  }

  void draw(const Image& image) {
    draw(image, {{0, 0}, image.dimensions()});
  }

  void draw(const Image& image, Box crop) {
    std::size_t starting_x = _position.x;
    for (std::size_t y = crop.position.y; y < crop.size.y; ++y) {
      for (std::size_t x = crop.position.x; x < crop.size.x; ++x) {
        _blend_pixel(image.pixel(x, y));
        _advance_column();
      }
      _advance_line();
      _position.x = starting_x;
    }
  }

  void blend_foreground() {
    for (Pixel& p : _buffer.pixels()) {
      p.fg = _alphablend(p.bg, p.fg);
      p.fg.a = 255;
    }
  }

private:
  void _advance_position() {
    ++_position.x;
    if (_position.x >= _buffer.width()) {
      _position.x = 0;
      _advance_line();
    }
  }

  void _advance_column() {
    ++_position.x;
    if (_position.x >= _buffer.width()) {
      _position.x = 0;
    }
  }

  void _advance_line() {
    ++_position.y;
    if (_position.y >= _buffer.height()) _position.y = 0;
  }

  void _blend_pixel(const Pixel& p) {
    Pixel& b = _buffer.pixel(_position);
    b.bg = _alphablend(b.bg, p.bg);
    b.fg = _alphablend(b.fg, p.fg);
    if (p.fg.a) b.glyph = p.glyph;
  }

  std::uint8_t _blend_channel(
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
      ((static_cast<double>(src) * alpha) + (static_cast<double>(dest) * (1.0 - alpha))) / out_alpha
    );
  }

  Color _alphablend(Color a, Color b) {
    a.r = _blend_channel(a.r, b.r, a.a, b.a);
    a.g = _blend_channel(a.g, b.g, a.a, b.a);
    a.b = _blend_channel(a.b, b.b, a.a, b.a);

    double b_alpha = b.a / 255.0;
    a.a = (b_alpha + ((a.a / 255.0) * (1.0 - b_alpha) )) * 255;
    return a;
  }

  Image _buffer;
  Color _background = Color::transparent();
  Color _foreground;
  UIVector2d _position = {0, 0};
};

class Renderable {
public:
  Renderable() = default;
  Renderable(const Renderable&) = default;
  Renderable(Renderable&&) = default;
  virtual ~Renderable() = default;
  Renderable& operator=(const Renderable&) = default;
  Renderable& operator=(Renderable&&) = default;

  virtual const Image& render(UIVector2d dimensions) = 0;
};

class Component: public Renderable {
public:
  explicit Component(UIVector2d dimensions): _renderer{dimensions} {}
  Component(const Component&) = delete;
  Component(Component&&) = default;
  virtual ~Component() = default;
  Component& operator=(const Component&) = delete;
  Component& operator=(Component&&) = default;

protected:
  Renderer& renderer() { return _renderer; };

private:
  Renderer _renderer;
};

class FrameBox: public Component {
public:
  struct CustomFrame {
    Pixel top_left;
    Pixel top_center;
    Pixel top_right;
    Pixel left;
    Pixel center;
    Pixel right;
    Pixel bottom_left;
    Pixel bottom_center;
    Pixel bottom_right;
  };
  struct SimpleFrame {
    char32_t top_left;
    char32_t top_right;
    char32_t bottom_left;
    char32_t bottom_right;
    char32_t horizontal;
    char32_t vertical;
    Color background;
    Color foreground;
  };

  FrameBox(CustomFrame frame, UIVector2d dimensions):
    Component{dimensions},
    _frame{std::move(frame)}
  {}

  FrameBox(SimpleFrame frame, UIVector2d dimensions):
    FrameBox{
      CustomFrame{
        .top_left = {frame.background, frame.foreground, frame.top_left},
        .top_center = {frame.background, frame.foreground, frame.horizontal},
        .top_right = {frame.background, frame.foreground, frame.top_right},
        .left = {frame.background, frame.foreground, frame.vertical},
        .center = {frame.background, Color::transparent(), U' '},
        .right = {frame.background, frame.foreground, frame.vertical},
        .bottom_left = {frame.background, frame.foreground, frame.bottom_left},
        .bottom_center = {frame.background, frame.foreground, frame.horizontal},
        .bottom_right = {
          frame.background,
          frame.foreground,
          frame.bottom_right
        },
      },
      dimensions
    }
  {}

  const Image& render(UIVector2d dimensions) override {
    // Calculate the size of the frame and contents.
    UIVector2d render_dims{
      std::min(renderer().image().width(), dimensions.x),
      std::min(renderer().image().height(), dimensions.y)
    };
    UIVector2d content_dims{render_dims.x - 2, render_dims.y - 2};
    if (render_dims.x < 3 || render_dims.y < 3) {
      renderer().clear({0xff00ff});
      return renderer().image();
    }

    // Draw the top of the frame.
    renderer().clear();
    renderer().move_to_start();
    renderer().draw(_frame.top_left);
    for (std::size_t i = 1; i < render_dims.x - 1; ++i) {
      renderer().draw(_frame.top_center);
    }
    renderer().draw(_frame.top_right);

    // Fill in the center of the contents. We'll only bother drawing the frame's
    // center before rendering the contents if the center has any opacity.
    if (_frame.center.bg.a > 0 || _frame.center.fg.a > 0) {
      for (std::size_t y = 1; y < render_dims.y - 1; ++y) {
        renderer().move_to({1, y});
        for (std::size_t x = 1; x < render_dims.x - 1; ++x) {
          renderer().draw(_frame.center);
        }
      }
    }
    if (_contents) {
      renderer().move_to({1, 1});
      renderer().draw(_contents->render(content_dims), {{0, 0}, content_dims});
    }

    // Draw the sides of the frame going down.
    for (std::size_t i = 1; i < render_dims.y - 1; ++i) {
      renderer().move_to({0, i});
      renderer().draw(_frame.left);
      renderer().move_to({render_dims.x - 1, i});
      renderer().draw(_frame.right);
    }

    // Draw the bottom of the frame.
    renderer().draw(_frame.bottom_left);
    for (std::size_t i = 1; i < render_dims.x - 1; ++i) {
      renderer().draw(_frame.bottom_center);
    }
    renderer().draw(_frame.bottom_right);
  }

private:
  CustomFrame _frame;
  std::shared_ptr<Renderable> _contents;
};

}

template <std::integral T>
constexpr T lerp(T a, T b, double x) {
  return a + (x * static_cast<double>(b - a));
}

template <std::floating_point T>
constexpr T lerp(T a, T b, T x) {
  return a + (x * (b - a));
}

void print_bg_gradient(
  lw::cli::Renderer& renderer,
  const lw::cli::Color& start,
  const lw::cli::Color& end,
  std::size_t width,
  char c
) {
  double step = 1.0 / static_cast<double>(width);
  for (std::size_t i = 0; i < width; ++i) {
    double x = step * i;
    lw::cli::Color color{
      lerp(start.r, end.r, x),
      lerp(start.g, end.g, x),
      lerp(start.b, end.b, x)
    };
    renderer.draw({color, lw::cli::Color::transparent(), static_cast<char32_t>(c)});
  }
}

void print_bg_gradient_sequence(
  lw::cli::Renderer& renderer,
  const std::vector<lw::cli::Color>& colors,
  std::size_t width,
  char c
) {
  double band_width = static_cast<double>(width) / static_cast<double>(colors.size() - 1);
  for (std::size_t i = 0; i < colors.size() - 2; ++i) {
    print_bg_gradient(renderer, colors[i], colors[i + 1], band_width, c);
  }
  // Last sequence needs to take up the slack.
  print_bg_gradient(
    renderer,
    colors[colors.size() - 2],
    colors.back(),
    width - (static_cast<std::size_t>(band_width) * (colors.size() - 2)),
    c
  );
}

void print_greyscale(lw::cli::Renderer& renderer) {
  print_bg_gradient(renderer, lw::cli::Color::transparent(), {0xffffff}, renderer.image().width(), ' ');
}

void print_rgb(lw::cli::Renderer& renderer) {
  print_bg_gradient(renderer, lw::cli::Color::transparent(), {0xff0000}, renderer.image().width(), ' ');
  print_bg_gradient(renderer, lw::cli::Color::transparent(), {0x00ff00}, renderer.image().width(), ' ');
  print_bg_gradient(renderer, lw::cli::Color::transparent(), {0x0000ff}, renderer.image().width(), ' ');
}

void print_rainbow(lw::cli::Renderer& renderer) {
  static const std::vector<lw::cli::Color> colors{
    {0xff0000}, {0xffff00}, {0x00ff00}, {0x00ffff}, {0x0000ff}, {0xff00ff}
  };
  print_bg_gradient_sequence(renderer, colors, renderer.image().width(), ' ');
}

void print_debug(lw::cli::Renderer& renderer) {
  for (std::size_t i = 0; i < renderer.image().height(); ++i) {
    print_rainbow(renderer);
  }
  renderer.move_to_start();

  print_greyscale(renderer);
  print_rgb(renderer);

  renderer.clear_draw_background();
  renderer.set_draw_color({0xffffff});
  for (std::size_t i = 0; i < renderer.image().width(); ++i) {
    renderer.draw(U'☃');
  }
  lw::cli::Color c{0xffffff};
  c.a = 0;
  double alpha = 0;
  double alpha_step = 255.0 / renderer.image().width();
  for (std::size_t i = 0; i < renderer.image().width(); ++i) {
    renderer.set_draw_color(c);
    renderer.draw(U'#');
    c.a = static_cast<std::uint8_t>(alpha += alpha_step);
  }
  for (std::size_t i = 0; i < renderer.image().width(); ++i) {
    renderer.set_draw_color(c);
    renderer.draw(U'#');
    c.a = static_cast<std::uint8_t>(alpha -= alpha_step);
  }

  renderer.move_to_start();
  alpha = 0;
  alpha_step = 255.0 / renderer.image().height();
  lw::cli::Pixel p{
    lw::cli::Color::transparent(),
    lw::cli::Color::transparent(),
    U'X'
  };
  for (std::size_t y = 0; y < renderer.image().height(); ++y) {
    for (std::size_t x = 0; x < renderer.image().width(); ++x) {
      renderer.draw(p);
    }
    p.bg.a = static_cast<std::uint8_t>(alpha += alpha_step);
  }
}

int main(int argc, char** argv) {
  auto terminal = lw::cli::Terminal::from_stdout();
  lw::cli::Renderer renderer{terminal->dimensions()};

  // TODO: SIGWINCH - signal terminal change (resize)

  std::vector<lw::cli::Renderer> ui_layers;
  ui_layers.emplace_back(lw::cli::UIVector2d{terminal->dimensions()});
  ui_layers.emplace_back(lw::cli::UIVector2d{terminal->dimensions()});

  print_debug(ui_layers.front());

  lw::cli::FrameBox frame{
    lw::cli::FrameBox::SimpleFrame{
      .top_left = U'╔',
      .top_right = U'╗',
      .bottom_left = U'╚',
      .bottom_right = U'╝',
      .horizontal = U'═',
      .vertical = U'║',
      .background = lw::cli::Color::transparent(),
      .foreground = {0x000000}
    },
    {10, 5}
  };
  ui_layers.front().move_to({5, 5});
  ui_layers.front().draw(frame.render({10, 5}));

  lw::cli::Pixel cursor{
    lw::cli::Color::transparent(),
    {0xff00ff},
    U'X'
  };
  lw::cli::UIVector2d cursor_pos{0, 0};
  lw::cli::Input i;
  do {
    ui_layers.back().clear();
    ui_layers.back().move_to(cursor_pos);
    ui_layers.back().draw(cursor);

    renderer.clear();
    for (const lw::cli::Renderer& layer : ui_layers) {
      renderer.draw(layer.image());
    }

    renderer.blend_foreground();
    terminal->clear_reset();
    terminal->print(renderer.image().pixels());

    i = terminal->read();

    if (i == lw::cli::Input::ARROW_UP && cursor_pos.y > 0) {
      --cursor_pos.y;
    } else if (i == lw::cli::Input::ARROW_DOWN && cursor_pos.y < terminal->height() - 1) {
      ++cursor_pos.y;
    } else if (i == lw::cli::Input::ARROW_LEFT && cursor_pos.x > 0) {
      --cursor_pos.x;
    } else if (i == lw::cli::Input::ARROW_RIGHT && cursor_pos.x < terminal->width() - 1) {
      ++cursor_pos.x;
    }
  } while (i != lw::cli::Input::ESCAPE);

  terminal->reset_style();
}
