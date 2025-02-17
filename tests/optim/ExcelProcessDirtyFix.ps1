# Desired number of Excel processes to maintain
$desiredCount = 4

while ($true) {
    # Get the list of running Excel processes
    $excelProcesses = Get-Process excel -ErrorAction SilentlyContinue

    # Check the number of running Excel processes
    $currentCount = $excelProcesses.Count

    if ($currentCount -gt $desiredCount) {
        # If there are more than the desired number, kill the extra processes
        $excessProcesses = $excelProcesses | Select-Object -First ($currentCount - $desiredCount)
        foreach ($process in $excessProcesses) {
            $process.Kill()
        }
    }

    # Wait 1 second before checking again
    Start-Sleep -Seconds 1
}
