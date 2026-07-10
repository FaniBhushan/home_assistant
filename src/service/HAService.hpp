#ifndef HA_SERVICE_HPP
#define HA_SERVICE_HPP

#include <boost/asio.hpp>

#include <atomic>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace service {

class HAService {
public:
  HAService();
  ~HAService();

  void init();
  void stop();

  /**
   * @brief Post a task to be executed by a worker thread.
   *
   * @param task The unit of work to execute.
   */
  void post(std::function<void()> task);

  boost::asio::io_context& ioContext();

private:
  void maybeGrowWorkersLocked();

  boost::asio::io_context m_ioContext;
  using WorkGuard =
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
  WorkGuard m_workGuard;
  std::mutex m_threadMutex;
  std::vector<std::thread> m_threads;
  std::atomic<std::size_t> m_pendingPostedTasks{0};
  unsigned int m_maxWorkerThreads;
  bool m_started{false};
};

} // namespace service
#endif // HA_SERVICE_HPP
