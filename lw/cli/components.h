#pragma once

#include <algorithm>
#include <memory>

#include "lw/cli/color.h"
#include "lw/cli/image.h"
#include "lw/cli/renderer.h"

namespace lw::cli {

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

  FrameBox(SimpleFrame frame, UIVector2d dimensions);

  const Image& render(UIVector2d dimensions) override;

private:
  CustomFrame _frame;
  std::shared_ptr<Renderable> _contents;
};

}
