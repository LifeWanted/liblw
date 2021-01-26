#pragma once

#include "lw/io/serializer/serializer.h"

namespace lw::io {
namespace testing {

struct ObjectTagged {
  int a;
  int b;
  int c;
};

struct ListTagged {
  int a;
  int b;
  int c;
};

}

template <>
struct Serialize<testing::ObjectTagged> {
  typedef ObjectTag serialization_category;

  void serialize(Serializer& serializer, const testing::ObjectTagged& value) {
    serializer.write("a", value.a);
    serializer.write("b", value.b);
    serializer.write("c", value.c);
  }
};

template <>
struct Serialize<testing::ListTagged> {
  typedef ListTag serialization_category;

  void serialize(Serializer& serializer, const testing::ListTagged& value) {
    serializer.write(value.a);
    serializer.write(value.b);
    serializer.write(value.c);
  }
};

}
