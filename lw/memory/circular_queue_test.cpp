#include "lw/memory/circular_queue.h"

#include "gtest/gtest.h"
#include "lw/err/canonical.h"

namespace lw {
namespace {

TEST(CircularQueue, PushPop) {
  CircularQueue<int> q{5};
  EXPECT_EQ(q.size(), 0);
  EXPECT_TRUE(q.empty());
  EXPECT_FALSE(q.full());

  EXPECT_TRUE(q.try_push_back(1));
  EXPECT_EQ(q.size(), 1);
  EXPECT_FALSE(q.empty());
  EXPECT_FALSE(q.full());

  q.push_back(2);
  EXPECT_EQ(q.size(), 2);
  EXPECT_FALSE(q.empty());
  EXPECT_FALSE(q.full());

  q.push_back(3);
  q.push_back(4);
  q.push_back(5);
  EXPECT_EQ(q.size(), 5);
  EXPECT_FALSE(q.empty());
  EXPECT_TRUE(q.full());

  EXPECT_FALSE(q.try_push_back(6));
  EXPECT_THROW(q.push_back(6), ResourceExhausted);

  EXPECT_EQ(q.pop_front(), 1);
  EXPECT_EQ(q.size(), 4);
  EXPECT_FALSE(q.empty());
  EXPECT_FALSE(q.full());

  EXPECT_EQ(q.pop_front(), 2);
  EXPECT_TRUE(q.try_push_back(6));
  EXPECT_EQ(q.size(), 4);
  EXPECT_FALSE(q.empty());
  EXPECT_FALSE(q.full());

  EXPECT_TRUE(q.try_push_back(7));
  EXPECT_EQ(q.size(), 5);
  EXPECT_FALSE(q.empty());
  EXPECT_TRUE(q.full());

  EXPECT_EQ(q.pop_front(), 3);
  EXPECT_EQ(q.pop_front(), 4);
  EXPECT_EQ(q.pop_front(), 5);
  EXPECT_EQ(q.pop_front(), 6);
  EXPECT_EQ(q.pop_front(), 7);

  EXPECT_EQ(q.size(), 0);
  EXPECT_TRUE(q.empty());
  EXPECT_FALSE(q.full());
}

}
}
