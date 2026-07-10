#include "service/HAService.hpp"

#include <utility>

namespace service {

HAService::HAService()
    : m_workGuard(boost::asio::make_work_guard(m_ioContext)),
      m_maxWorkerThreads(std::thread::hardware_concurrency() == 0
                             ? 2
                             : std::thread::hardware_concurrency()) {}

HAService::~HAService() { stop(); }

void HAService::init() {
  std::lock_guard<std::mutex> lock(m_threadMutex);
  if (m_started) {
    return;
  }

  m_started = true;
  m_threads.emplace_back([this]() { m_ioContext.run(); });
}

void HAService::stop() {
  m_workGuard.reset(); // Allow run() to exit when valid work is done
  m_ioContext.stop();  // Explicitly stop to interrupt long running handlers if
                       // necessary

  std::vector<std::thread> threads;
  {
    std::lock_guard<std::mutex> lock(m_threadMutex);
    m_started = false;
    threads.swap(m_threads);
  }

  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void HAService::post(std::function<void()> task) {
  init();
  m_pendingPostedTasks.fetch_add(1, std::memory_order_relaxed);

  {
    std::lock_guard<std::mutex> lock(m_threadMutex);
    maybeGrowWorkersLocked();
  }

  boost::asio::post(m_ioContext, [this, task = std::move(task)]() mutable {
    struct PendingTaskGuard {
      std::atomic<std::size_t>& pendingTasks;
      ~PendingTaskGuard() {
        pendingTasks.fetch_sub(1, std::memory_order_relaxed);
      }
    } guard{m_pendingPostedTasks};

    task();
  });
}

boost::asio::io_context& HAService::ioContext() { return m_ioContext; }

void HAService::maybeGrowWorkersLocked() {
  while (m_threads.size() < m_maxWorkerThreads &&
         m_pendingPostedTasks.load(std::memory_order_relaxed) >
             m_threads.size()) {
    m_threads.emplace_back([this]() { m_ioContext.run(); });
  }
}

} // namespace service
