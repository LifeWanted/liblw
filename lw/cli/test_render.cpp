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
#include "lw/base/math.h"
#include "lw/cli/color.h"
#include "lw/cli/components.h"
#include "lw/cli/image.h"
#include "lw/cli/input.h"
#include "lw/cli/renderer.h"
#include "lw/cli/terminal.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"

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
      lw::lerp(start.r, end.r, x),
      lw::lerp(start.g, end.g, x),
      lw::lerp(start.b, end.b, x)
    };
    renderer.draw(
      {color, lw::cli::Color::transparent(), static_cast<char32_t>(c)}
    );
  }
}

void print_bg_gradient_sequence(
  lw::cli::Renderer& renderer,
  const std::vector<lw::cli::Color>& colors,
  std::size_t width,
  char c
) {
  double band_width =
    static_cast<double>(width) / static_cast<double>(colors.size() - 1);
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
  print_bg_gradient(
    renderer,
    lw::cli::Color::transparent(), {0xffffff},
    renderer.image().width(), ' '
  );
}

void print_rgb(lw::cli::Renderer& renderer) {
  print_bg_gradient(
    renderer,
    lw::cli::Color::transparent(), {0xff0000},
    renderer.image().width(), ' '
  );
  print_bg_gradient(
    renderer,
    lw::cli::Color::transparent(), {0x00ff00},
    renderer.image().width(), ' '
  );
  print_bg_gradient(
    renderer,
    lw::cli::Color::transparent(), {0x0000ff},
    renderer.image().width(), ' '
  );
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

  lw::cli::FrameBox frame{lw::cli::FrameBox::BOLD_BAR_FRAME, {10, 5}};
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

    // Adjust cursor from inputs.
    if (i == lw::cli::Input::ARROW_UP && cursor_pos.y > 0) {
      --cursor_pos.y;
    } else if (
      i == lw::cli::Input::ARROW_DOWN &&
      cursor_pos.y < terminal->height() - 1
    ) {
      ++cursor_pos.y;
    } else if (
      i == lw::cli::Input::ARROW_LEFT &&
      cursor_pos.x > 0
    ) {
      --cursor_pos.x;
    } else if (
      i == lw::cli::Input::ARROW_RIGHT &&
      cursor_pos.x < terminal->width() - 1
    ) {
      ++cursor_pos.x;
    }
  } while (i != lw::cli::Input::ESCAPE);

  terminal->reset_style();
}
