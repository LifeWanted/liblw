#pragma once

#include <memory>
#include <span>

#include "lw/cli/color.h"
#include "lw/cli/image.h"
#include "lw/cli/input.h"

namespace lw::cli {

/**
 * Direct-draw terminal control. All commands are printed straight to the linked
 * terminal stream.
 */
class Terminal {
public:
  Terminal(const Terminal&) = delete;
  Terminal& operator=(const Terminal&) = delete;

  Terminal(Terminal&& other);
  Terminal& operator=(Terminal&& other);

  ~Terminal();

  static std::unique_ptr<Terminal> from_fd(int fd);
  static std::unique_ptr<Terminal> from_stdout();

  std::size_t width() const { return _win_size.x; }
  std::size_t height() const { return _win_size.y; }
  UIVector2d dimensions() const { return _win_size; }

  /**
   * Moves the print cursor to the specified location.
   */
  void move_to(std::size_t x, std::size_t y);
  void move_to_start();
  void move_to_end() {
    move_to(width() - 1, height() - 1);
  }

  void reset_style();
  void clear_reset();

  void set_fg(const Color& color);
  void set_bg(const Color& color);

  /**
   * Prints the given character to the window and advances the print cursor 1
   * position.
   */
  void print(char c);
  void print(char32_t c);
  void print(std::span<const Pixel> pixels);

  /**
   * Resets the terminal attributes to what they were before this `Terminal`
   * instance took control of it. This is automatically called upon destruction
   * of a `Terminal`.
   */
  void reset_terminal_attributes();

  Input read();

private:
  Terminal(int fd, UIVector2d win_size, void* attributes):
    _fd{fd},
    _win_size{std::move(win_size)},
    _starting_attributes{attributes}
  {}

  int _fd;
  UIVector2d _win_size;
  void* _starting_attributes = nullptr;
};

}
