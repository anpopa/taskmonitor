-- Get system PSI usage statistics
SELECT
    datetime(SystemTime,'unixepoch') as 'SystemTime',
	datetime(ReceiveTime,'unixepoch') as 'ReceiveTime',
	MonotonicTime as 'MonotonicTime',
	CPUSomeAvg10 as 'CPUSomeAvg10',
	MEMSomeAvg10 as 'MEMSomeAvg10',
	IOSomeAvg10 as 'IOSomeAvg10'
FROM
	tkmSysProcPressure
-- Condition for a specific session
	-- WHERE SessionId IS (SELECT Id FROM tkmSessions WHERE Name IS 'Collector.100363.1670947874')