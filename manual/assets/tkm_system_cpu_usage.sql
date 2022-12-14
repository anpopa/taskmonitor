-- Get system CPU usage statistics
SELECT
    datetime(SystemTime,'unixepoch') as 'SystemTime',
	datetime(ReceiveTime,'unixepoch') as 'ReceiveTime',
	MonotonicTime as 'MonotonicTime',
	CPUStatAll as 'Total',
	CPUStatUsr as 'User',
	CPUStatSys as 'System',
	CPUStatIow as 'IoWait'
FROM
	tkmSysProcStat
WHERE
    -- Change to core name to get results per core eg 'cpu0'
	CPUStatName IS 'cpu'
	-- Condition for a specific session
		-- AND SessionId IS (SELECT Id FROM tkmSessions WHERE Name IS 'Collector.100363.1670947874')