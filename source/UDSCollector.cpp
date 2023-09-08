/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     UDSCollector Class
 * @details   Network UDS collector implementation
 *-
 */

#include <taskmonitor/taskmonitor.h>

#include "Application.h"
#include "UDSCollector.h"

namespace tkm::monitor
{

static bool doCreateSession(const std::shared_ptr<UDSCollector> collector);
static bool doGetProcAcct(const std::shared_ptr<UDSCollector> collector);
static bool doGetProcInfo(const std::shared_ptr<UDSCollector> collector);
static bool doGetProcEventStats(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcMemInfo(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcDiskStats(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcStat(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcPressure(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcBuddyInfo(const std::shared_ptr<UDSCollector> collector);
static bool doGetSysProcWireless(const std::shared_ptr<UDSCollector> collector);
static bool doGetContextInfo(const std::shared_ptr<UDSCollector> collector);
#ifdef WITH_VM_STAT
static bool doGetSysProcVMStat(const std::shared_ptr<UDSCollector> collector);
#endif

UDSCollector::UDSCollector(int fd)
: ICollector("UDSCollector", ICollector::Type::UDS, fd)
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
          case tkm::msg::collector::Request_Type_GetContextInfo:
            status = doGetContextInfo(getShared());
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
          case tkm::msg::collector::Request_Type_GetSysProcBuddyInfo:
            status = doGetSysProcBuddyInfo(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcWireless:
            status = doGetSysProcWireless(getShared());
            break;
          case tkm::msg::collector::Request_Type_GetSysProcVMStat:
#ifdef WITH_VM_STAT
            status = doGetSysProcVMStat(getShared());
#else
            status = true;
#endif
            break;
          case tkm::msg::collector::Request_Type_KeepAlive:
            status = true;
            break;
          default:
            logDebug() << "Unknown type " << collectorMessage.type();
            status = false;
            break;
          }

          if (status) {
            setLastUpdateTime(std::chrono::steady_clock::now());
          }
        } while (status);

        return status;
      },
      getFD(),
      bswi::event::IPollable::Events::Level,
      bswi::event::IEventSource::Priority::Normal);

  // For UDSCollector we don't sent ProcAcct so we skip incProcAcctCollectorCounter()
  setFinalize([this]() {
    logInfo() << "Ended connection with collector: " << getFD();

    // The collector event source is about to be removed.
    // Request state manager to remove this collector without event source removal.
    StateManager::Request rq = {.action = StateManager::Action::RemoveCollector,
                                .collector = getShared()};
    App()->getStateManager()->pushRequest(rq);
  });
}

void UDSCollector::setEventSource(bool enabled)
{
  if (enabled) {
    App()->addEventSource(getShared());
  } else {
    App()->remEventSource(getShared());
  }
}

UDSCollector::~UDSCollector()
{
  logDebug() << "UDSCollector " << getFD() << " destructed";
}

static bool doCreateSession(const std::shared_ptr<UDSCollector> collector)
{
  tkm::msg::Envelope envelope;
  tkm::msg::monitor::Message message;
  std::string idContent(collector->getDescriptor().id());

  char randData[64] = {0};
  srandom(static_cast<unsigned int>(time(0)));
  snprintf(randData, sizeof(randData), "%0lx", static_cast<unsigned long>(random()));
  idContent += randData;

  logInfo() << "Session hash content: " << idContent
            << " jenkinsHash: " << tkm::jnkHsh(idContent.c_str());
  collector->getDescriptor().set_id(std::to_string(tkm::jnkHsh(idContent.c_str())));
  collector->getSessionInfo().set_libtkm_version(TKMLIB_VERSION);
  collector->getSessionInfo().set_hash(collector->getDescriptor().id());
  collector->getSessionInfo().set_core_count(static_cast<uint32_t>(sysconf(_SC_NPROCESSORS_ONLN)));
  logDebug() << "Send new sessionID=" << collector->getSessionInfo().hash();

  auto keepAliveInterval =
      std::stoul(App()->getOptions()->getFor(Options::Key::CollectorInactiveTimeout));
  collector->getSessionInfo().set_keep_alive_interval(keepAliveInterval);

  collector->getSessionInfo().set_fast_lane_interval(App()->getFastLaneInterval());
  collector->getSessionInfo().set_pace_lane_interval(App()->getPaceLaneInterval());
  collector->getSessionInfo().set_slow_lane_interval(App()->getSlowLaneInterval());

  collector->getSessionInfo().add_pace_lane_sources(msg::monitor::SessionInfo_DataSource_ProcInfo);
  collector->getSessionInfo().add_pace_lane_sources(
      msg::monitor::SessionInfo_DataSource_ContextInfo);
#ifdef WITH_PROC_EVENT
  if (App()->getProcEvent() != nullptr) {
    collector->getSessionInfo().add_pace_lane_sources(
        msg::monitor::SessionInfo_DataSource_ProcEvent);
  }
#endif
#ifdef WITH_PROC_ACCT
  if (App()->getProcAcct() != nullptr) {
    collector->getSessionInfo().add_slow_lane_sources(
        msg::monitor::SessionInfo_DataSource_ProcAcct);
  }
#endif
  if (App()->getSysProcStat() != nullptr) {
    collector->getSessionInfo().add_fast_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcStat);
  }
  if (App()->getSysProcMemInfo() != nullptr) {
    collector->getSessionInfo().add_fast_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcMemInfo);
  }
  if (App()->getSysProcPressure() != nullptr) {
    collector->getSessionInfo().add_pace_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcPressure);
  }
  if (App()->getSysProcDiskStats() != nullptr) {
    collector->getSessionInfo().add_pace_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcDiskStats);
  }
  if (App()->getSysProcBuddyInfo() != nullptr) {
    collector->getSessionInfo().add_slow_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcBuddyInfo);
  }
  if (App()->getSysProcWireless() != nullptr) {
    collector->getSessionInfo().add_slow_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcWireless);
  }
#ifdef WITH_VM_STAT
  if (App()->getSysProcVMStat() != nullptr) {
    collector->getSessionInfo().add_slow_lane_sources(
        msg::monitor::SessionInfo_DataSource_SysProcVMStat);
  }
#endif

  message.set_type(tkm::msg::monitor::Message::Type::Message_Type_SetSession);
  message.mutable_payload()->PackFrom(collector->getSessionInfo());

  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Collector);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Monitor);

  logDebug() << "Send session id: " << collector->getSessionInfo().hash()
             << " to collector: " << collector->getFD();
  return collector->writeEnvelope(envelope);
}

static bool doGetProcAcct(const std::shared_ptr<UDSCollector> collector)
{
#ifdef WITH_PROC_ACCT
  if (App()->getProcAcct() != nullptr) {
    ProcRegistry::Request rq = {.action = ProcRegistry::Action::CollectAndSendProcAcct,
                                .collector = collector};
    return App()->getProcRegistry()->pushRequest(rq);
  }
  return true;
#else
  static_cast<void>(collector);
  return true;
#endif
}

static bool doGetProcInfo(const std::shared_ptr<UDSCollector> collector)
{
  ProcRegistry::Request rq = {.action = ProcRegistry::Action::CollectAndSendProcInfo,
                              .collector = collector};
  return App()->getProcRegistry()->pushRequest(rq);
}

static bool doGetSysProcWireless(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcWireless() != nullptr) {
    SysProcWireless::Request rq = {.action = SysProcWireless::Action::CollectAndSend,
                                   .collector = collector};
    return App()->getSysProcWireless()->pushRequest(rq);
  }
  return true;
}

static bool doGetProcEventStats(const std::shared_ptr<UDSCollector> collector)
{
#ifdef WITH_PROC_EVENT
  if (App()->getProcEvent() != nullptr) {
    ProcEvent::Request rq = {.action = ProcEvent::Action::CollectAndSend, .collector = collector};
    return App()->getProcEvent()->pushRequest(rq);
  }
  return true;
#else
  static_cast<void>(collector);
  return true;
#endif
}

static bool doGetSysProcMemInfo(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcMemInfo() != nullptr) {
    SysProcMemInfo::Request rq = {.action = SysProcMemInfo::Action::CollectAndSend,
                                  .collector = collector};
    return App()->getSysProcMemInfo()->pushRequest(rq);
  }
  return true;
}

static bool doGetSysProcDiskStats(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcDiskStats() != nullptr) {
    SysProcDiskStats::Request rq = {.action = SysProcDiskStats::Action::CollectAndSend,
                                    .collector = collector};
    return App()->getSysProcDiskStats()->pushRequest(rq);
  }
  return true;
}

static bool doGetSysProcStat(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcStat() != nullptr) {
    SysProcStat::Request rq = {.action = SysProcStat::Action::CollectAndSend,
                               .collector = collector};
    return App()->getSysProcStat()->pushRequest(rq);
  }
  return true;
}

static bool doGetSysProcPressure(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcPressure() != nullptr) {
    SysProcPressure::Request rq = {.action = SysProcPressure::Action::CollectAndSend,
                                   .collector = collector};
    return App()->getSysProcPressure()->pushRequest(rq);
  }
  return true;
}

static bool doGetSysProcBuddyInfo(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcBuddyInfo() != nullptr) {
    SysProcBuddyInfo::Request rq = {.action = SysProcBuddyInfo::Action::CollectAndSend,
                                    .collector = collector};
    return App()->getSysProcBuddyInfo()->pushRequest(rq);
  }
  return true;
}

static bool doGetContextInfo(const std::shared_ptr<UDSCollector> collector)
{
  ProcRegistry::Request rq = {.action = ProcRegistry::Action::CollectAndSendContextInfo,
                              .collector = collector};
  return App()->getProcRegistry()->pushRequest(rq);
}

#ifdef WITH_VM_STAT
static bool doGetSysProcVMStat(const std::shared_ptr<UDSCollector> collector)
{
  if (App()->getSysProcVMStat() != nullptr) {
    SysProcVMStat::Request rq = {.action = SysProcVMStat::Action::CollectAndSend,
                                    .collector = collector};
    return App()->getSysProcVMStat()->pushRequest(rq);
  }
  return true;
}
#endif

} // namespace tkm::monitor
