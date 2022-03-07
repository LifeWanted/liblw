#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <memory>

#include "lw/cli/color.h"
#include "lw/cli/image.h"
#include "lw/cli/renderer.h"

namespace lw::cli {

/**
 * @brief Base class for drawable elements.
 */
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

/**
 * @brief A dimensioned, renderable UI component.
 */
class Component: public Renderable {
public:
  explicit Component(UIVector2d dimensions): _renderer{dimensions} {}
  Component(const Component&) = delete;
  Component(Component&&) = default;
  ~Component() override = default;
  Component& operator=(const Component&) = delete;
  Component& operator=(Component&&) = default;

protected:
  /**
   * @brief Direct access to renderer used for this component.
   */
  Renderer& renderer() { return _renderer; };

private:
  Renderer _renderer;
};

/**
 * @brief A UI component with a rendered frame surrounding the contents.
 */
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

  static const SimpleFrame DOUBLE_BAR_FRAME;
  static const SimpleFrame SINGLE_BAR_FRAME;
  static const SimpleFrame BOLD_BAR_FRAME;

  explicit FrameBox(CustomFrame frame, UIVector2d dimensions):
    Component{dimensions},
    _frame{std::move(frame)}
  {}

  explicit FrameBox(SimpleFrame frame, UIVector2d dimensions);
  ~FrameBox() override = default;

  const Image& render(UIVector2d dimensions) override;

private:
  CustomFrame _frame;
  std::shared_ptr<Renderable> _contents;
};

class StaticTextBox: public Component {
public:
  explicit StaticTextBox(std::string_view text, UIVector2d dimensions):
    Component{dimensions},
    _text{text}
  {}
  ~StaticTextBox() override = default;

  const Image& render(UIVector2d dimensions) override;

private:
  std::string _text;
};

}
