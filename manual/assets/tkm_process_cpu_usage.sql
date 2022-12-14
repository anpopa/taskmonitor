-- Get process CPU usage statistics
SELECT
    datetime(SystemTime,'unixepoch') as 'SystemTime',
	datetime(ReceiveTime,'unixepoch') as 'ReceiveTime',
	MonotonicTime as 'MonotonicTime',
	CPUTime as 'Time',
	CPUPercent as 'Percent'
FROM
	tkmProcInfo
WHERE
    -- Change to process name to get results for your process
	Comm IS 'taskmonitor'
	-- Condition for specific instance (PID)
		-- AND PID IS '3630'
	-- Condition for a specific session
		-- AND SessionId IS (SELECT Id FROM tkmSessions WHERE Name IS 'Collector.100363.1670947874')