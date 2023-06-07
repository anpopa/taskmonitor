-- Get process Memory usage statistics in KB
SELECT
    datetime(SystemTime,'unixepoch') as 'SystemTime',
	datetime(ReceiveTime,'unixepoch') as 'ReceiveTime',
	MonotonicTime as 'MonotonicTime',
	MemRSS as 'RSS',
	MemPSS as 'PSS',
	MemShared as 'Shared'
FROM
	tkmProcInfo
WHERE
    -- Change to process name to get results for your process
	Comm IS 'taskmonitor'
	-- Condition for specific instance (PID)
		-- AND PID IS '3630'
	-- Condition for a specific session
		-- AND SessionId IS (SELECT Id FROM tkmSessions WHERE Name IS 'Collector.100363.1670947874')
