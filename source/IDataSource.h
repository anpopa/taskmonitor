/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IDataSource Class
 * @details   Data source Interface
 *-
 */

#pragma once

#include <cstdint>
#include <string>

namespace tkm::monitor
{

class IDataSource
{
public:
  typedef enum _UpdateLane { Fast, Pace, Slow, Any } UpdateLane;

public:
  IDataSource() = default;
  ~IDataSource() = default;

public:
  IDataSource(IDataSource const &) = delete;
  void operator=(IDataSource const &) = delete;

public:
  auto getUpdateInterval() -> uint64_t { return m_updateInterval; };
  void setUpdateInterval(uint64_t interval)
  {
    if (interval >= 1000000) {
      m_updateInterval = interval;
    }
  }
  auto getUpdateLane(void) -> UpdateLane { return m_updateLane; }
  void setUpdateLane(UpdateLane lane) { m_updateLane = lane; }
  virtual bool update(const std::string &) { return update(); };
  virtual bool update(UpdateLane) { return update(); };
  virtual bool update(void) = 0;

protected:
  bool getUpdatePending(void) { return m_updatePending; }
  void setUpdatePending(bool state) { m_updatePending = state; }

protected:
  uint64_t m_updateInterval = 1000000;
  bool m_updatePending = false;
  UpdateLane m_updateLane = UpdateLane::Fast;
  int m_fd = -1;
};

} // namespace tkm::monitor
