#pragma once

#include "lw/io/serializer/serializer.h"
#include "lw/io/serializer/serialized_value.h"

namespace lw::io {
namespace testing {

struct ObjectTagged {
  int a;
  int b;
  int c;
};

bool operator==(const ObjectTagged& lhs, const ObjectTagged& rhs) {
  return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c;
}

struct ListTagged {
  int a;
  int b;
  int c;
};

bool operator==(const ListTagged& lhs, const ListTagged& rhs) {
  return lhs.a == rhs.a && lhs.b == rhs.b && lhs.c == rhs.c;
}

}

template <>
struct Serialize<testing::ObjectTagged> {
  typedef ObjectTag serialization_category;

  void serialize(Serializer& serializer, const testing::ObjectTagged& value) {
    serializer.write("a", value.a);
    serializer.write("b", value.b);
    serializer.write("c", value.c);
  }

  testing::ObjectTagged deserialize(const SerializedValue& value) {
    return testing::ObjectTagged{
      .a = value.get<int>("a"),
      .b = value.get<int>("b"),
      .c = value.get<int>("c")
    };
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

  testing::ListTagged deserialize(const SerializedValue& value) {
    return testing::ListTagged{
      .a = value.get<int>(0),
      .b = value.get<int>(1),
      .c = value.get<int>(2)
    };
  }
};

}
