# Task Monitor

Monitor and report performance indicators for embedded systems

## Description

Task monitor is a system daemon that collects performance indicators and sends the data out from the system for processing. 
As part of the project group a set of tools is provided to store and process the output data.

### Components
| Component | Location | Description |
| ------ | ------ | ------ |
| monitor | https://gitlab.com/taskmonitor/taskmonitor | System daemon running on target system (this project) |
| interfaces | https://gitlab.com/taskmonitor/taskmonitor-interfaces | Protobuf interfaces to communicate with the daemon |
| reader | https://gitlab.com/taskmonitor/taskmonitor-reader | A simple reader that output data in JSON format |
| collector | https://gitlab.com/taskmonitor/taskmonitor-collector | A data collector, SQL based, to be used in CI systems |

## Download
`# git clone --recurse-submodules https://gitlab.com/taskmonitor/taskmonitor.git`

## Dependencies

Taskmonitor depends on the following libraries

| Library | Reference | Info |
| ------ | ------ | ------ |
| libnl3 | https://www.infradead.org/~tgr/libnl | Used for netlink interfaces |
| protobuf | https://developers.google.com/protocol-buffers | Data serialization |
| libsystemd | https://github.com/systemd/systemd/tree/main/src/libsystemd | Optional if WITH_SYSTEMD is ON |

## Build
### Compile options

| Option | Default | Info |
| ------ | ------ | ------ |
| WITH_SYSTEMD | ON | Enable systemd service and watchdog support |

### Local Build
`mkdir build && cd build && cmake .. && make `

## Sample syslog output
```
Mar 26 04:54:32 rpi4dev systemd[1]: Started Task Monitor Service.
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: NetServer listening on port: 3357
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Systemd watchdog enabled with timeout 10 seconds
Mar 26 04:54:32 rpi4dev TaskMonitor[2128]: Opt proc at init true
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

## Sample tkmreader output
```
...
{"cpu":{"full":{"avg10":"","avg300":"","avg60":"","total":""},"some":{"avg10":"0.00","avg300":"0.02","avg60":"0.00","total":"1701451"}},"device":"rpi4dev","io":{"full":{"avg10":"0.03","avg300":"0.31","avg60":"0.25","total":"4972076"},"some":{"avg10":"0.03","avg300":"0.33","avg60":"0.25","total":"5520131"}},"lifecycle":"na","mem":{"full":{"avg10":"0.00","avg300":"0.00","avg60":"0.00","total":"0"},"some":{"avg10":"0.00","avg300":"0.00","avg60":"0.00","total":"0"}},"session":"2715708755170151292","time":1648665374,"type":"psi"}
{"cpu":{"all":1,"sys":0,"usr":1},"device":"rpi4dev","lifecycle":"na","session":"2715708755170151292","time":1648665374,"type":"stat"}
{"common":{"ac_comm":"systemd","ac_gid":1000,"ac_pid":575,"ac_ppid":1,"ac_stime":28000,"ac_uid":1000,"ac_utime":160000,"sys_cpu_percent":0,"user_cpu_percent":0},"cpu":{"cpu_count":54,"cpu_delay_average":0,"cpu_delay_total":189642038,"cpu_run_real_total":188000000,"cpu_run_virtual_total":189642038},"ctx":{"nivcsw":18,"nvcsw":36},"device":"rpi4dev","io":{"blkio_count":30,"blkio_delay_average":0,"blkio_delay_total":23302566},"lifecycle":"na","mem":{"coremem":1258437,"hiwater_rss":7824,"hiwater_vm":15892,"virtmem":5214828},"reclaim":{"freepages_count":0,"freepages_delay_average":0,"freepages_delay_total":0},"session":"2715708755170151292","swap":{"nivcsw":18,"nvcsw":36},"thrashing":{"thrashing_count":0,"thrashing_delay_average":0,"thrashing_delay_total":0},"time":1648665374,"type":"acct"}
{"common":{"ac_comm":"(sd-pam)","ac_gid":1000,"ac_pid":576,"ac_ppid":575,"ac_stime":0,"ac_uid":1000,"ac_utime":0,"sys_cpu_percent":0,"user_cpu_percent":0},"cpu":{"cpu_count":1,"cpu_delay_average":0,"cpu_delay_total":702148,"cpu_run_real_total":0,"cpu_run_virtual_total":702148},"ctx":{"nivcsw":0,"nvcsw":1},"device":"rpi4dev","io":{"blkio_count":0,"blkio_delay_average":0,"blkio_delay_total":0},"lifecycle":"na","mem":{"coremem":0,"hiwater_rss":4224,"hiwater_vm":168616,"virtmem":0},"reclaim":{"freepages_count":0,"freepages_delay_average":0,"freepages_delay_total":0},"session":"2715708755170151292","swap":{"nivcsw":0,"nvcsw":1},"thrashing":{"thrashing_count":0,"thrashing_delay_average":0,"thrashing_delay_total":0},"time":1648665374,"type":"acct"}
{"common":{"ac_comm":"sshd","ac_gid":1000,"ac_pid":598,"ac_ppid":572,"ac_stime":96000,"ac_uid":1000,"ac_utime":52000,"sys_cpu_percent":0,"user_cpu_percent":0},"cpu":{"cpu_count":680,"cpu_delay_average":0,"cpu_delay_total":124124473,"cpu_run_real_total":148000000,"cpu_run_virtual_total":124124473},"ctx":{"nivcsw":28,"nvcsw":652},"device":"rpi4dev","io":{"blkio_count":0,"blkio_delay_average":0,"blkio_delay_total":0},"lifecycle":"na","mem":{"coremem":646921,"hiwater_rss":4476,"hiwater_vm":16244,"virtmem":2347765},"reclaim":{"freepages_count":0,"freepages_delay_average":0,"freepages_delay_total":0},"session":"2715708755170151292","swap":{"nivcsw":28,"nvcsw":652},"thrashing":{"thrashing_count":0,"thrashing_delay_average":0,"thrashing_delay_total":0},"time":1648665374,"type":"acct"}
{"common":{"ac_comm":"fish","ac_gid":1000,"ac_pid":599,"ac_ppid":598,"ac_stime":184000,"ac_uid":1000,"ac_utime":224000,"sys_cpu_percent":0,"user_cpu_percent":0},"cpu":{"cpu_count":844,"cpu_delay_average":0,"cpu_delay_total":435099499,"cpu_run_real_total":408000000,"cpu_run_virtual_total":435099499},"ctx":{"nivcsw":137,"nvcsw":707},"device":"rpi4dev","io":{"blkio_count":77,"blkio_delay_average":1,"blkio_delay_total":102676977},"lifecycle":"na","mem":{"coremem":2794312,"hiwater_rss":9620,"hiwater_vm":250092,"virtmem":82639749},"reclaim":{"freepages_count":0,"freepages_delay_average":0,"freepages_delay_total":0},"session":"2715708755170151292","swap":{"nivcsw":137,"nvcsw":707},"thrashing":{"thrashing_count":0,"thrashing_delay_average":0,"thrashing_delay_total":0},"time":1648665374,"type":"acct"}
{"common":{"ac_comm":"taskmonitor","ac_gid":0,"ac_pid":973,"ac_ppid":1,"ac_stime":28000,"ac_uid":0,"ac_utime":60000,"sys_cpu_percent":0,"user_cpu_percent":0},"cpu":{"cpu_count":70,"cpu_delay_average":0,"cpu_delay_total":97564636,"cpu_run_real_total":88000000,"cpu_run_virtual_total":97564636},"ctx":{"nivcsw":3,"nvcsw":66},"device":"rpi4dev","io":{"blkio_count":0,"blkio_delay_average":0,"blkio_delay_total":0},"lifecycle":"na","mem":{"coremem":507546,"hiwater_rss":7460,"hiwater_vm":13360,"virtmem":1701093},"reclaim":{"freepages_count":0,"freepages_delay_average":0,"freepages_delay_total":0},"session":"2715708755170151292","swap":{"nivcsw":3,"nvcsw":66},"thrashing":{"thrashing_count":0,"thrashing_delay_average":0,"thrashing_delay_total":0},"time":1648665374,"type":"acct"}
{"cpu":{"all":0,"sys":0,"usr":0},"device":"rpi4dev","lifecycle":"na","session":"2715708755170151292","time":1648665375,"type":"stat"}
{"cpu":{"all":0,"sys":0,"usr":0},"device":"rpi4dev","lifecycle":"na","session":"2715708755170151292","time":1648665376,"type":"stat"}
{"cpu":{"all":0,"sys":0,"usr":0},"device":"rpi4dev","lifecycle":"na","session":"2715708755170151292","time":1648665377,"type":"stat"}
{"cpu":{"all":0,"sys":0,"usr":0},"device":"rpi4dev","lifecycle":"na","session":"2715708755170151292","time":1648665378,"type":"stat"}
{"device":"rpi4dev","exit":{"exit_code":0,"process_pid":337,"process_tgid":337},"lifecycle":"na","session":"2715708755170151292","time":1648665378,"type":"proc"}
{"device":"rpi4dev","exit":{"exit_code":0,"process_pid":159,"process_tgid":159},"lifecycle":"na","session":"2715708755170151292","time":1648665378,"type":"proc"}
{"device":"rpi4dev","exit":{"exit_code":0,"process_pid":160,"process_tgid":160},"lifecycle":"na","session":"2715708755170151292","time":1648665379,"type":"proc"}
...
```

