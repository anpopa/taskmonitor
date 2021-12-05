# TaskMonitor

Monitor and report performance indicators for embedded systems

## Build
mkdir build && cd build    
cmake ..    
make     

## Sample output
```
TaskMonitor|Info   	| Logging open for TaskMonitor
TaskMonitor|Debug  	| Request task accounting for pid 125568
TaskMonitor|Info   	| MON::COMMON[125568] Command=taskmonitor UID=0 GID=0 PID=125568 PPID=125562 UserCPUTime=1955 SystemCPUTime=995
TaskMonitor|Info   	| MON::CPU[125568] Count=4 RealTotal=2951094 VirtualTotal=2035430 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::MEMORY[125568] CoreMem=1330MB-usec VirtMem=11996MB-usec HiWaterRSS=2088KBytes HiWaterVM=6216KBytes
TaskMonitor|Info   	| MON::CONTEXT[125568] Voluntary=3 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Debug  	| Request task accounting for pid 125568
TaskMonitor|Info   	| MON::COMMON[125568] Command=taskmonitor UID=0 GID=0 PID=125568 PPID=125562 UserCPUTime=1955 SystemCPUTime=995
TaskMonitor|Info   	| MON::CPU[125568] Count=7 RealTotal=2951094 VirtualTotal=2491784 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::MEMORY[125568] CoreMem=1330MB-usec VirtMem=11996MB-usec HiWaterRSS=2088KBytes HiWaterVM=6216KBytes
TaskMonitor|Info   	| MON::CONTEXT[125568] Voluntary=6 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Debug  	| Request task accounting for pid 125568
TaskMonitor|Info   	| MON::COMMON[125568] Command=taskmonitor UID=0 GID=0 PID=125568 PPID=125562 UserCPUTime=2911 SystemCPUTime=995
TaskMonitor|Info   	| MON::CPU[125568] Count=10 RealTotal=3907104 VirtualTotal=2902365 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::MEMORY[125568] CoreMem=5236MB-usec VirtMem=23623MB-usec HiWaterRSS=2088KBytes HiWaterVM=6216KBytes
TaskMonitor|Info   	| MON::CONTEXT[125568] Voluntary=9 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[125568] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[125568] Count=0 DelayTotal=0 DelayAverage=0
^CTaskMonitor|Info   	| Received signal 2
```
