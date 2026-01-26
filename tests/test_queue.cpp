#include "accelsim/queue.h"
#include <gtest/gtest.h>

TEST(BoundedQueue, FIFOAndCapacity) {
  accelsim::BoundedQueue<int> q(2);
  EXPECT_TRUE(q.empty());
  q.push(1);
  q.push(2);
  EXPECT_TRUE(q.full());
  EXPECT_EQ(q.pop(), 1);
  EXPECT_EQ(q.pop(), 2);
  EXPECT_TRUE(q.empty());
}
