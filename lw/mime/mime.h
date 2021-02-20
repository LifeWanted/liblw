#pragma once

#include <memory>
#include <ostream>

#include "lw/io/serializer/formatter.h"

namespace lw::mime {

class MimeSerializer {
public:
  virtual std::unique_ptr<io::SerializationFormatter> make_formatter(
    std::ostream& output
  ) = 0;

  virtual std::unique_ptr<io::DeserializationParser> make_parser() = 0;
};

}
