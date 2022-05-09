# TaskMonitor
Monitor and report performance indicators from embedded systems

## Description
**TaskMonitor** is a system daemon that collects performance indicators and sends the data out from the system for processing. 
As part of the project group a set of tools is provided to store and process the output data.

### Components
| Component | Location | Description |
| ------ | ------ | ------ |
| taskmonitor | https://gitlab.com/taskmonitor/taskmonitor | System daemon running on target system (this project) |
| taskmonitor-interfaces | https://gitlab.com/taskmonitor/taskmonitor-interfaces | Protobuf interfaces to communicate with the daemon |
| tkm-reader | https://gitlab.com/taskmonitor/tkm-reader | A simple reader that output data in SQLite and JSON formats |
| tkm-collector | https://gitlab.com/taskmonitor/tkm-collector | A data collector, SQL based, to be used in CI systems |

## Download
Clone this repository with submodules:    
`# git clone --recurse-submodules https://gitlab.com/taskmonitor/taskmonitor.git`

## Dependencies
TaskMonitor depends on the following libraries

| Library | Reference | Info |
| ------ | ------ | ------ |
| libnl3 | https://www.infradead.org/~tgr/libnl | Used for netlink interfaces to taskstats |
| protobuf | https://developers.google.com/protocol-buffers | Data serialization |
| libsystemd | https://github.com/systemd/systemd/tree/main/src/libsystemd | Optional if WITH_SYSTEMD is ON |

## Build
### Build time options

| Option | Default | Info |
| ------ | ------ | ------ |
| WITH_SYSTEMD | ON | Enable systemd service, watchdog and journald support |
| WITH_INSTALL_CONFIG | ON | Install default taskmonitor.conf on target |
| WITH_INSTALL_LICENSE | ON | Install license file on target for QA checks |

### Local Build
`mkdir build && cd build && cmake .. && make `

## Execute
The service needs elevated capabilities.    
To start in manually:    
`sudo taskmonitor -c /etc/taskmonitor.conf`    
As a systemd service if WITH_SYSTEMD is ON:    
`sudo systemctl enable --now taskmonitor.service`

## Sample syslog output
```
Mar 26 04:54:32 rpi4dev systemd[1]: Started Task Monitor Service.
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: NetServer listening on port: 3357
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Systemd watchdog enabled with timeout 10 seconds
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Read existing proc entries
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 1
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 2
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 3
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 4
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 8
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 9
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 10
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 11
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 12
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 13
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 15
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 16
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 17
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 18
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Add process monitoring for pid 21
...

```
