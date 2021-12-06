# TaskMonitor

Monitor and report performance indicators for embedded systems

## Build
mkdir build && cd build    
cmake ..    
make     

## Sample output
```
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164678] Count=1 RealTotal=994842ns VirtualTotal=1127685ns DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164678] CoreMem=0MB-usec VirtMem=0MB-usec HiWaterRSS=1352KBytes HiWaterVM=144128KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164678] Voluntary=1 NonVoluntary=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164678] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164688] Command=gpg-agent UID=0 GID=0 PID=164688 PPID=1 UserCPUTime=76000 SystemCPUTime=160950
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164688] Count=8495 RealTotal=236950675ns VirtualTotal=401140624ns DelayTotal=14120135 DelayAverage=0.00166217
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164688] CoreMem=198075MB-usec VirtMem=52234665MB-usec HiWaterRSS=856KBytes HiWaterVM=225736KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164688] Voluntary=8492 NonVoluntary=3
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164688] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164690] Command=scdaemon UID=0 GID=0 PID=164690 PPID=164688 UserCPUTime=0 SystemCPUTime=997
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164690] Count=1 RealTotal=997586ns VirtualTotal=1002569ns DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164690] CoreMem=0MB-usec VirtMem=0MB-usec HiWaterRSS=1372KBytes HiWaterVM=144128KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164690] Voluntary=1 NonVoluntary=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164690] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164701] Command=gpg-agent UID=0 GID=0 PID=164701 PPID=1 UserCPUTime=60983 SystemCPUTime=157000
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164701] Count=8493 RealTotal=217983094ns VirtualTotal=400156059ns DelayTotal=15492760 DelayAverage=0.00182418
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164701] CoreMem=189883MB-usec VirtMem=48053351MB-usec HiWaterRSS=892KBytes HiWaterVM=225736KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164701] Voluntary=8491 NonVoluntary=2
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164701] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164703] Command=scdaemon UID=0 GID=0 PID=164703 PPID=164701 UserCPUTime=0 SystemCPUTime=994
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164703] Count=1 RealTotal=994706ns VirtualTotal=1150710ns DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164703] CoreMem=0MB-usec VirtMem=0MB-usec HiWaterRSS=1340KBytes HiWaterVM=144128KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164703] Voluntary=1 NonVoluntary=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164703] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164714] Command=gpg-agent UID=0 GID=0 PID=164714 PPID=1 UserCPUTime=64000 SystemCPUTime=161970
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164714] Count=8493 RealTotal=225970910ns VirtualTotal=400155872ns DelayTotal=14027231 DelayAverage=0.00165162
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164714] CoreMem=225087MB-usec VirtMem=49814227MB-usec HiWaterRSS=1020KBytes HiWaterVM=225736KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164714] Voluntary=8491 NonVoluntary=2
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164714] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164716] Command=scdaemon UID=0 GID=0 PID=164716 PPID=164714 UserCPUTime=0 SystemCPUTime=990
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164716] Count=1 RealTotal=990434ns VirtualTotal=1145650ns DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164716] CoreMem=0MB-usec VirtMem=0MB-usec HiWaterRSS=1368KBytes HiWaterVM=144128KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164716] Voluntary=1 NonVoluntary=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164716] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164732] Command=gpg-agent UID=0 GID=0 PID=164732 PPID=1 UserCPUTime=66961 SystemCPUTime=185000
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164732] Count=8492 RealTotal=251961823ns VirtualTotal=403547584ns DelayTotal=10803620 DelayAverage=0.00127221
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164732] CoreMem=216068MB-usec VirtMem=55047465MB-usec HiWaterRSS=884KBytes HiWaterVM=225736KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164732] Voluntary=8491 NonVoluntary=1
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164732] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164734] Command=scdaemon UID=0 GID=0 PID=164734 PPID=164732 UserCPUTime=0 SystemCPUTime=996
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164734] Count=1 RealTotal=996765ns VirtualTotal=1417866ns DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164734] CoreMem=0MB-usec VirtMem=0MB-usec HiWaterRSS=1380KBytes HiWaterVM=144128KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164734] Voluntary=1 NonVoluntary=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::IO[164734] Count=0 DelayTotal=0 DelayAverage=0
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::COMMON[164748] Command=gpg-agent UID=0 GID=0 PID=164748 PPID=1 UserCPUTime=64995 SystemCPUTime=165976
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CPU[164748] Count=8491 RealTotal=230971726ns VirtualTotal=403117844ns DelayTotal=15825158 DelayAverage=0.00186376
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::MEMORY[164748] CoreMem=224640MB-usec VirtMem=50852634MB-usec HiWaterRSS=996KBytes HiWaterVM=225736KBytes
Dec 06 15:44:51 xps15 taskmonitor[211433]: MON::CONTEXT[164748] Voluntary=8490 NonVoluntary=1

```
