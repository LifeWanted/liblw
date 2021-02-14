#pragma once

#include <variant>

namespace lw::cli {

class Input {
public:
  enum Escape {
    NUL                 = 0x00,
    START_OF_HEADING    = 0x01,
    START_OF_TEXT       = 0x02,
    END_OF_TEXT         = 0x03,
    END_OF_TRANSMISSION = 0x04,
    ENQUIRY             = 0x05,
    ACKNOWLEDGE         = 0x06,
    BELL                = 0x07,
    BACKSPACE           = 0x08,
    TAB                 = 0x09,
    LINE_FEED           = 0x0a,
    VERTICAL_TAB        = 0x0b,
    NEW_PAGE            = 0x0c,
    CARRIAGE_RETURN     = 0x0d,
    SHIFT_OUT           = 0x0e,
    SHIFT_IN            = 0x0f,
    // TODO(#11): Fill in this chart.
    ESCAPE              = 0x1b, // 27 / 033 -> ESC character

    ARROW_UP            = 0x001b5b41,
    ARROW_DOWN          = 0x001b5b42,
    ARROW_RIGHT         = 0x001b5b43,
    ARROW_LEFT          = 0x001b5b44
  };

  Input(): _value{NUL} {}
  Input(const Input&) = default;
  Input(Input&&) = default;
  ~Input() = default;
  Input& operator=(const Input&) = default;
  Input& operator=(Input&&) = default;

  explicit Input(Escape esc): _value{esc} {}
  explicit Input(char c): _value{static_cast<char32_t>(c)} {}
  explicit Input(char32_t c): _value{c} {}

  bool is_escape() const {
    return std::holds_alternative<Escape>(_value);
  }

  bool is_glyph() const {
    return std::holds_alternative<char32_t>(_value);
  }

  Escape as_escape() const {
    return std::get<Input::Escape>(_value);
  }

  char32_t as_glyph() const {
    return std::get<char32_t>(_value);
  }

private:
  std::variant<Escape, char32_t> _value;
};

inline bool operator==(const Input& input, Input::Escape esc) {
  return input.is_escape() && input.as_escape() == esc;
}
inline bool operator==(Input::Escape esc, const Input& input) {
  return input == esc;
}

inline bool operator!=(const Input& input, Input::Escape esc) {
  return !(input == esc);
}
inline bool operator!=(Input::Escape esc, const Input& input) {
  return input != esc;
}

inline bool operator==(const Input& input, char32_t glyph) {
  return input.is_glyph() && input.as_glyph() == glyph;
}
inline bool operator==(char32_t glyph, const Input& input) {
  return input == glyph;
}

inline bool operator!=(const Input& input, char32_t glyph) {
  return !(input == glyph);
}
inline bool operator!=(char32_t glyph, const Input& input) {
  return input != glyph;
}

}
