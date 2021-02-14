#include "lw/cli/terminal.h"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <memory>

#include "lw/cli/color.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"

namespace lw::cli {

Terminal::Terminal(Terminal&& other):
  _fd{other._fd},
  _win_size{other._win_size},
  _starting_attributes{std::move(other._starting_attributes)}
{
  other._fd = 0;
  other._win_size = {0, 0};
}

Terminal& Terminal::operator=(Terminal&& other) {
  _fd = other._fd;
  _win_size = other._win_size;
  _starting_attributes = std::move(other._starting_attributes);
  other._fd = 0;
  other._win_size = {0, 0};
  return *this;
}

Terminal::~Terminal() {
  if (_fd) reset_terminal_attributes();
  if (_starting_attributes) {
    delete static_cast<::termios*>(_starting_attributes);
  }
}

std::unique_ptr<Terminal> Terminal::from_fd(int fd) {
  ::winsize w{0};
  if (::ioctl(fd, TIOCGWINSZ, &w) != 0) {
    check_system_error();
    throw Internal() << "Unknown ioctl error fetching terminal size.";
  }

  auto attributes = std::make_unique<::termios>();
  if (::tcgetattr(fd, attributes.get()) != 0) {
    check_system_error();
    throw Internal() << "Unknown termios error fetching terminal attributes.";
  }

  ::termios raw_mode{*attributes};
  ::cfmakeraw(&raw_mode);
  if (::tcsetattr(fd, TCSAFLUSH, &raw_mode) != 0) {
    check_system_error();
    throw Internal() << "Unknown termios error setting terminal attributes.";
  }

  return std::unique_ptr<Terminal>{
    new Terminal{fd, {w.ws_col, w.ws_row}, attributes.release()}
  };
}

std::unique_ptr<Terminal> Terminal::from_stdout() {
  return from_fd(STDOUT_FILENO);
}

void Terminal::move_to(std::size_t x, std::size_t y) {
  char buff[32];
  int written = std::snprintf(
    buff, sizeof(buff),
    "\033[%lu;%luH",
    y + 1, x + 1
  );
  ::write(_fd, buff, written);
}

void Terminal::move_to_start() {
  ::write(_fd, "\033[H", 3);
}

void Terminal::reset_style() {
  ::write(_fd, "\033[0m", 4);
}

void Terminal::clear_reset() {
  ::write(_fd, "\033[H\033[J", 10);
}

void Terminal::set_fg(const Color& color) {
  char buff[32];
  int written = std::snprintf(
    buff, sizeof(buff),
    "\033[38;2;%hhu;%hhu;%hhum",
    color.r, color.g, color.b
  );
  ::write(_fd, buff, written);
}

void Terminal::set_bg(const Color& color) {
  char buff[32];
  int written = std::snprintf(
    buff, sizeof(buff),
    "\033[48;2;%hhu;%hhu;%hhum",
    color.r, color.g, color.b
  );
  ::write(_fd, buff, written);
}

void Terminal::print(char c) {
  ::write(_fd, &c, 1);
}

void Terminal::print(char32_t c) {
  if (c < 0x80) {
    print(static_cast<char>(c));
  } else if (c < 0x0800) {
    char buff[2] = {
      static_cast<char>(0xc0 | (0x1f & (c >> 6))),
      static_cast<char>(0x80 | (0x3f & c))
    };
    ::write(_fd, buff, 2);
  } else if (c < 0x010000) {
    char buff[3] = {
      static_cast<char>(0xe0 | (0x0f & (c >> 12))),
      static_cast<char>(0x80 | (0x3f & (c >>  6))),
      static_cast<char>(0x80 | (0x3f & c))
    };
    ::write(_fd, buff, 3);
  } else if (c < 0x110000) {
    char buff[4] = {
      static_cast<char>(0xf0 | (0x07 & (c >> 18))),
      static_cast<char>(0x80 | (0x3f & (c >> 12))),
      static_cast<char>(0x80 | (0x3f & (c >>  6))),
      static_cast<char>(0x80 | (0x3f & c))
    };
    ::write(_fd, buff, 4);
  } else {
    print('?');
  }
}

void Terminal::print(std::span<const Pixel> pixels) {
  Color fg = pixels.front().fg;
  Color bg = pixels.front().bg;
  set_fg(fg);
  set_bg(bg);
  for (const Pixel& p : pixels) {
    if (bg != p.bg) set_bg(bg = p.bg);
    if (fg != p.fg) set_fg(fg = p.fg);
    print(p.glyph);
  }
}

void Terminal::reset_terminal_attributes() {
  int reset_result = ::tcsetattr(
    _fd,
    TCSAFLUSH,
    static_cast<::termios*>(_starting_attributes)
  );
  if (reset_result != 0) {
    check_system_error();
    throw Internal() << "Unknown termios error resetting attributes.";
  }
}

Input Terminal::read() {
  char buff[16] = {0};
  std::size_t bytes_read = ::read(_fd, &buff, sizeof(buff));
  if (bytes_read == 1) {
    if (std::isprint(buff[0])) {
      return Input{buff[0]};
    }
    return Input{static_cast<Input::Escape>(buff[0])};
  }

  if (buff[0] == Input::ESCAPE) {
    constexpr std::size_t ESCAPE_SIZE = sizeof(Input::Escape);
    char esc[ESCAPE_SIZE] = {0};
    for (std::size_t i = 0; i < bytes_read && i < ESCAPE_SIZE; ++i) {
      esc[bytes_read - i - 1] = buff[i];
    }
    return Input{*reinterpret_cast<Input::Escape*>(esc)};
  }

  // TODO: UTF-8 to UTF-32 conversion here.
  return Input{buff[0]};
}

}
