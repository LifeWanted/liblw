#pragma once

#include <string_view>

#include "lw/io/co/co.h"
#include "lw/io/co/testing/string_readable.h"

namespace lw::io::testing {

class StringReader: public CoReader<StringReadable> {
public:
  explicit StringReader(std::string_view str):
    CoReader<StringReadable>{_readable},
    _readable{str}
  {}

private:
  StringReadable _readable;
};

}
