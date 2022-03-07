#include "lw/cli/components.h"

#include "lw/cli/color.h"

namespace lw::cli {

// https://en.wikipedia.org/wiki/Box-drawing_character#Box_Drawing
const FrameBox::SimpleFrame FrameBox::DOUBLE_BAR_FRAME{
  .top_left = U'╔',
  .top_right = U'╗',
  .bottom_left = U'╚',
  .bottom_right = U'╝',
  .horizontal = U'═',
  .vertical = U'║',
  .background = lw::cli::Color::transparent(),
  .foreground = {0x000000}
};
const FrameBox::SimpleFrame FrameBox::SINGLE_BAR_FRAME{
  .top_left = U'┌',
  .top_right = U'┐',
  .bottom_left = U'└',
  .bottom_right = U'┘',
  .horizontal = U'─',
  .vertical = U'│',
  .background = lw::cli::Color::transparent(),
  .foreground = {0x000000}
};
const FrameBox::SimpleFrame FrameBox::BOLD_BAR_FRAME{
  .top_left = U'┏',
  .top_right = U'┓',
  .bottom_left = U'┗',
  .bottom_right = U'┛',
  .horizontal = U'━',
  .vertical = U'┃',
  .background = lw::cli::Color::transparent(),
  .foreground = {0x000000}
};

FrameBox::FrameBox(SimpleFrame frame, UIVector2d dimensions):
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

const Image& FrameBox::render(UIVector2d dimensions) {
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

  return renderer().image();
}

}
