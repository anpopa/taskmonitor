/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     TCPCollector Class
 * @details   Network TCP collector implementation
 *-
 */

#include <unistd.h>

#include "Application.h"
#include "Defaults.h"
#include "Dispatcher.h"
#include "Helpers.h"
#include "TCPCollector.h"

#include "Collector.pb.h"

namespace tkm::monitor
{

static bool doCreateSession(const std::shared_ptr<TCPCollector> collector);
static bool doGetProcAcct(const std::shared_ptr<TCPCollector> collector);
static bool doGetProcInfo(const std::shared_ptr<TCPCollector> collector);
static bool doGetProcEventStats(const std::shared_ptr<TCPCollector> collector);
static bool doGetSysProcMemInfo(const std::shared_ptr<TCPCollector> collector);
static bool doGetSysProcDiskStats(const std::shared_ptr<TCPCollector> collector);
static bool doGetSysProcStat(const std::shared_ptr<TCPCollector> collector);
static bool doGetSysProcPressure(const std::shared_ptr<TCPCollector> collector);
static bool doGetContextInfo(const std::shared_ptr<TCPCollector> collector);

TCPCollector::TCPCollector(int fd)
: ICollector("TCPCollector", fd)
{
  bswi::event::Pollable::lateSetup(
      [this]() {
        auto status = true;

        do {
          tkm::msg::Envelope envelope;

          // Read next message
          auto readStatus = readEnvelope(envelope);
          if (readStatus == IAsyncEnvelope::Status::Again) {
            return true;
          } else if (readStatus == IAsyncEnvelope::Status::Error) {
            logDebug() << "Collector read error";
            return false;
          } else if (readStatus == IAsyncEnvelope::Status::EndOfFile) {
            logDebug() << "Collector read end of file";
            return false;
          }

          // Handle generic collector request
          if (envelope.origin() != msg::Envelope_Recipient_Collector) {
            ::close(getFD());
            return false;
          }

          tkm::msg::collector::Request collectorMessage;
          envelope.mesg().UnpackTo(&collectorMessage);

          status = true;
          switch (collectorMessage.type()) {
          case tkm::msg::collector::Request_Type_CreateSession:
            status = doCreateSession(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetProcAcct:
            status = doGetProcAcct(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetProcInfo:
            status = doGetProcInfo(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetProcEventStats:
            status = doGetProcEventStats(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcMemInfo:
            status = doGetSysProcMemInfo(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcDiskStats:
            status = doGetSysProcDiskStats(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcStat:
            status = doGetSysProcStat(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcPressure:
            status = doGetSysProcPressure(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetContextInfo:
            status = doGetContextInfo(getShared());
            break;
          default:
            logDebug() << "Unknown type " << collectorMessage.type();
            status = false;
            break;
          }
        } while (status);

        return status;
      },
      getFD(),
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  setFinalize([this]() {
    logInfo() << "Ended connection with collector: " << getFD();
    App()->decProcAcctCollectorCounter();
  });
  App()->incProcAcctCollectorCounter();
}

void TCPCollector::enableEvents()
{
  App()->addEventSource(getShared());
}

TCPCollector::~TCPCollector()
{
  logDebug() << "TCPCollector " << getFD() << " destructed";
  if (getFD() > 0) {
    ::close(getFD());
  }
}

static bool doCreateSession(const std::shared_ptr<TCPCollector> collector)
{
  tkm::msg::Envelope envelope;
  tkm::msg::monitor::Message message;
  tkm::msg::monitor::SessionInfo sessionInfo;
  std::string idContent(collector->descriptor.id());

  char randData[64] = {0};
  srandom(time(0));
  snprintf(randData, sizeof(randData), "%0lX", random());
  idContent += randData;

  logInfo() << "Session hash content: " << idContent
            << " jenkinsHash: " << tkm::jnkHsh(idContent.c_str());
  collector->id = std::to_string(tkm::jnkHsh(idContent.c_str()));
  sessionInfo.set_hash(collector->id);
  logInfo() << "Send new sessionID=" << sessionInfo.hash();

  // TODO: Don't know how to get LC ID yet
  sessionInfo.set_lifecycle_id("na");
  try {
    sessionInfo.set_proc_acct_poll_interval(App()->getSlowLaneInterval());
    sessionInfo.set_proc_info_poll_interval(App()->getPaceLaneInterval());
    sessionInfo.set_proc_event_poll_interval(App()->getPaceLaneInterval());
    sessionInfo.set_sys_proc_stat_poll_interval(App()->getFastLaneInterval());
    sessionInfo.set_sys_proc_meminfo_poll_interval(App()->getFastLaneInterval());
    sessionInfo.set_sys_proc_diskstats_poll_interval(App()->getPaceLaneInterval());
    sessionInfo.set_sys_proc_pressure_poll_interval(App()->getPaceLaneInterval());
    sessionInfo.set_context_information_poll_interval(App()->getPaceLaneInterval());
  } catch (...) {
    throw std::runtime_error("Fail to process session poll interval data");
  }

  message.set_type(tkm::msg::monitor::Message::Type::Message_Type_SetSession);
  message.mutable_payload()->PackFrom(sessionInfo);

  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Monitor);

  logDebug() << "Send session id: " << sessionInfo.hash()
             << " to collector: " << collector->getFD();
  return collector->writeEnvelope(envelope);
}

static bool doGetProcAcct(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetProcAcct, .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetProcInfo(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetProcInfo, .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetProcEventStats(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetProcEventStats,
                             .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetSysProcMemInfo(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetSysProcMemInfo,
                             .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetSysProcDiskStats(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetSysProcDiskStats,
                             .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetSysProcStat(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetSysProcStat, .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetSysProcPressure(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetSysProcPressure,
                             .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

static bool doGetContextInfo(const std::shared_ptr<TCPCollector> collector)
{
  Dispatcher::Request req = {.action = Dispatcher::Action::GetContextInfo, .collector = collector};
  return App()->getDispatcher()->pushRequest(req);
}

} // namespace tkm::monitor
