-- Get system Memory usage statistics (in MB)
SELECT
    datetime(SystemTime,'unixepoch') as 'SystemTime',
	datetime(ReceiveTime,'unixepoch') as 'ReceiveTime',
	MonotonicTime as 'MonotonicTime',
	round(MemTotal / 1024, 2) as 'Total',
	round(MemFree / 1024, 2) as 'Free',
	round(MemAvail / 1024, 2) as 'Available',
	MemAvailPercent as 'Available%',
	round(SwapTotal / 1024, 2) as 'SWAP Total',
	round(SwapFree / 1024, 2) as 'SWAP Free',
	round(CmaTotal / 1024, 2) as 'CMA Total',
	round(CmaFree / 1024, 2) as 'CMA Free'
FROM
	tkmSysProcMemInfo
-- Condition for a specific session
	-- WHERE SessionId IS (SELECT Id FROM tkmSessions WHERE Name IS 'Collector.100363.1670947874')