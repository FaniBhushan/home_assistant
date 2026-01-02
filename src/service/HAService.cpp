#include "service/HAService.hpp"

namespace service {

HAService::HAService()
    : m_workGuard(boost::asio::make_work_guard(m_ioContext)) {}

HAService::~HAService() { stop(); }

void HAService::init() {
  if (!m_threads.empty()) {
    return; // Already initialized
  }

  unsigned int threadCount = std::thread::hardware_concurrency();
  if (threadCount == 0)
    threadCount = 2; // Fallback

  m_threads.reserve(threadCount);
  for (unsigned int i = 0; i < threadCount; ++i) {
    m_threads.emplace_back([this]() { m_ioContext.run(); });
  }
}

void HAService::stop() {
  m_workGuard.reset(); // Allow run() to exit when valid work is done
  m_ioContext.stop();  // Explicitly stop to interrupt long running handlers if
                       // necessary

  for (auto &thread : m_threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  m_threads.clear();
}

void HAService::post(std::function<void()> task) {
  boost::asio::post(m_ioContext, std::move(task));
}

} // namespace service
