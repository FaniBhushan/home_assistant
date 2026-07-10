#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "service/HAService.hpp"

TEST(HAServiceTest, PostExecutesTask) {
  service::HAService haService;

  std::mutex mutex;
  std::condition_variable cv;
  bool executed = false;

  haService.post([&]() {
    {
      std::lock_guard<std::mutex> lock(mutex);
      executed = true;
    }
    cv.notify_one();
  });

  {
    std::unique_lock<std::mutex> lock(mutex);
    const bool finished =
        cv.wait_for(lock, std::chrono::seconds(2), [&]() { return executed; });
    EXPECT_TRUE(finished);
  }

  haService.stop();
}
