# Developer Manual
This manual will provide some hacking information into TaskMonitor.

## Introduction
The system is devided in 3 major components:
1. On device data collector (taskmonitor service)
2. Data collector (tkmreader or tkmcollector on host)
3. Data visualization (Grafana, SQLiteBrowser, TkmViewer, etc)

### Dataflow
* The taskmonitor service is running on target device and collects data from procfs or netlink interfaces. 
* This data is serialized over a TCP socket using *protobuf* for a data collector.
* The data collector is storing the data using common data storage formats like SQL database or JSON files.
* To facilitate IPC between taskmonitor service and data collector *libtaskmonitor* is provided: [libtaskmonitor](https://gitlab.com/taskmonitor/libtaskmonitor)

### LibTaskMonitor
This library provides data types and the interfaces for IPC between the service and collector.
* The interfaces are defined using Google protobuf: [Data Types](https://gitlab.com/taskmonitor/libtaskmonitor/-/tree/main/proto)
* Read/Write classes are provided by the library for raw sockets: [IPC Helpers](https://gitlab.com/taskmonitor/libtaskmonitor/-/tree/main/include)