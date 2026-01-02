#ifndef HA_SERVICE_HPP
#define HA_SERVICE_HPP

#include <boost/asio.hpp>
#include <functional>
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

private:
  boost::asio::io_context m_ioContext;
  using WorkGuard =
      boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
  WorkGuard m_workGuard;
  std::vector<std::thread> m_threads;
};

} // namespace service
#endif // HA_SERVICE_HPP
