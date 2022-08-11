/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     SysProcDiskStats Class
 * @details   Collect and report information from /proc/diskstats
 *-
 */

#include "SysProcDiskStats.h"
#include "Application.h"

namespace tkm::monitor
{

static bool doUpdateStats(const std::shared_ptr<SysProcDiskStats> mgr,
                          const SysProcDiskStats::Request &request);
static bool doCollectAndSend(const std::shared_ptr<SysProcDiskStats> mgr,
                             const SysProcDiskStats::Request &request);

SysProcDiskStats::SysProcDiskStats(const std::shared_ptr<Options> options)
: m_options(options)
{
  m_queue = std::make_shared<AsyncQueue<Request>>(
      "SysProcDiskStatsQueue", [this](const Request &request) { return requestHandler(request); });
}

auto SysProcDiskStats::pushRequest(Request &request) -> int
{
  return m_queue->push(request);
}

void SysProcDiskStats::enableEvents()
{
  App()->addEventSource(m_queue);
}

bool SysProcDiskStats::update()
{
  if (getUpdatePending()) {
    return true;
  }

  SysProcDiskStats::Request request = {.action = SysProcDiskStats::Action::UpdateStats};
  bool status = pushRequest(request);

  if (status) {
    setUpdatePending(true);
  }

  return status;
}

auto SysProcDiskStats::requestHandler(const Request &request) -> bool
{
  bool status = false;

  switch (request.action) {
  case SysProcDiskStats::Action::UpdateStats:
    status = doUpdateStats(getShared(), request);
    setUpdatePending(false);
    break;
  case SysProcDiskStats::Action::CollectAndSend:
    status = doCollectAndSend(getShared(), request);
    break;
  default:
    logError() << "Unknown action request";
    break;
  }

  return status;
}

static bool doUpdateStats(const std::shared_ptr<SysProcDiskStats> mgr,
                          const SysProcDiskStats::Request &request)
{
  std::ifstream diskStatsStream{"/proc/diskstats"};

  if (!diskStatsStream.is_open()) {
    throw std::runtime_error("Fail to open /proc/diskstats file");
  }

  std::string line;
  while (std::getline(diskStatsStream, line)) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string buf;

    while (ss >> buf) {
      tokens.push_back(buf);
    }

    if (tokens.size() < 20) {
      logError() << "Proc diskstats file parse error";
      return false;
    }

    auto major = std::stoul(tokens[0]);
    auto minor = std::stoul(tokens[1]);

    auto updateDiskStatEntry = [&tokens, &major, &minor](const std::shared_ptr<DiskStat> &entry) {
      entry->getData().set_major(major);
      entry->getData().set_minor(minor);
      entry->getData().set_name(tokens[2]);
      entry->getData().set_reads_completed(std::stoul(tokens[3]));
      entry->getData().set_reads_merged(std::stoul(tokens[4]));
      entry->getData().set_reads_spent_ms(std::stoul(tokens[6]));
      entry->getData().set_writes_completed(std::stoul(tokens[7]));
      entry->getData().set_writes_merged(std::stoul(tokens[8]));
      entry->getData().set_writes_spent_ms(std::stoul(tokens[10]));
      entry->getData().set_io_in_progress(std::stoul(tokens[11]));
      entry->getData().set_io_spent_ms(std::stoul(tokens[12]));
      entry->getData().set_io_weighted_ms(std::stoul(tokens[13]));
    };

    auto found = false;
    mgr->getDiskStatList().foreach ([&tokens, &found, &major, &minor, updateDiskStatEntry](
                                        const std::shared_ptr<DiskStat> &entry) {
      if ((entry->getData().major() == major) && (entry->getData().minor() == minor)) {
        updateDiskStatEntry(entry);
        found = true;
      }
    });

    if (!found) {
      std::shared_ptr<DiskStat> entry =
          std::make_shared<DiskStat>(tokens[2], std::stoul(tokens[0]), std::stoul(tokens[1]));

      logInfo() << "Adding new diskstat entry '" << entry->getData().name() << "' for statistics";
      mgr->getDiskStatList().append(entry);
      mgr->getDiskStatList().commit();
      updateDiskStatEntry(entry);
    }
  }

  return true;
}

static bool doCollectAndSend(const std::shared_ptr<SysProcDiskStats> mgr,
                             const SysProcDiskStats::Request &request)
{
  tkm::msg::monitor::SysProcDiskStats diskStats;
  tkm::msg::monitor::Data data;

  data.set_what(tkm::msg::monitor::Data_What_SysProcDiskStats);

  struct timespec currentTime;
  clock_gettime(CLOCK_REALTIME, &currentTime);
  data.set_system_time_sec(currentTime.tv_sec);
  clock_gettime(CLOCK_MONOTONIC, &currentTime);
  data.set_monotonic_time_sec(currentTime.tv_sec);

  mgr->getDiskStatList().foreach ([&diskStats](const std::shared_ptr<DiskStat> &entry) {
    diskStats.add_disk()->CopyFrom(entry->getData());
  });

  data.mutable_payload()->PackFrom(diskStats);
  request.collector->sendData(data);

  return true;
}

} // namespace tkm::monitor
