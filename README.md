# TaskMonitor

Monitor and report performance indicators for embedded systems

## Build
mkdir build && cd build    
cmake ..    
make     

## Sample output
```
TaskMonitor|Info   	| Logging open for TaskMonitor
TaskMonitor|Debug  	| Request task accounting for pid 123963
TaskMonitor|Info   	| MON::CPU[123963] Count=4 RealTotal=1989253 VirtualTotal=2090749 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::CONTEXT[123963] Voluntary=3 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Debug  	| Request task accounting for pid 123963
TaskMonitor|Info   	| MON::CPU[123963] Count=7 RealTotal=1989253 VirtualTotal=2514656 DelayTotal=17082 DelayAverage=0.00244029
TaskMonitor|Info   	| MON::CONTEXT[123963] Voluntary=6 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Debug  	| Request task accounting for pid 123963
TaskMonitor|Info   	| MON::CPU[123963] Count=10 RealTotal=1989253 VirtualTotal=2902027 DelayTotal=17082 DelayAverage=0.0017082
TaskMonitor|Info   	| MON::CONTEXT[123963] Voluntary=9 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Debug  	| Request task accounting for pid 123963
TaskMonitor|Info   	| MON::CPU[123963] Count=13 RealTotal=1989253 VirtualTotal=3222428 DelayTotal=17082 DelayAverage=0.001314
TaskMonitor|Info   	| MON::CONTEXT[123963] Voluntary=12 NonVoluntary=0
TaskMonitor|Info   	| MON::IO[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::SWAP[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::RECLAIM[123963] Count=0 DelayTotal=0 DelayAverage=0
TaskMonitor|Info   	| MON::THRASHING[123963] Count=0 DelayTotal=0 DelayAverage=0
^CTaskMonitor|Info   	| Received signal 2
```
