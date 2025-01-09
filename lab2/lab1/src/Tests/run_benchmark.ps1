param (
    [string]$OutputLogDir,
    [string]$SortProgram,
    [string]$IoThptWriteProgram,
    [int]$Repetitions = 1,  # Количество повторений
    [string[]]$SortParam,   # Параметры для sort программы
    [string[]]$IoThptWriteParam  # Параметры для io-thpt-write программы
)

# Создание каталога для логов
if (!(Test-Path -Path $OutputLogDir)) {
    New-Item -ItemType Directory -Path $OutputLogDir | Out-Null
}

# Конвертируем аргументы в строку для передачи программам
$sortArguments = $SortParam -join " "
$ioArguments = $IoThptWriteParam -join " "

# Функция для сбора статистики процесса
function Collect-ProcessStats {
    param (
        [System.Diagnostics.Process]$Process,
        [string]$LogFile
    )

    # Время начала выполнения
    $startTime = Get-Date
    $cpuUsagePrev = 0
    $sysTimePrev = 0
    $userTimePrev = 0
    $contextSwitchesPrev = 0

    while (-not $Process.HasExited) {
        try {
            # Получение статистики процессора и памяти с использованием Process API
            $cpuUsage = $Process.TotalProcessorTime.TotalMilliseconds
            $sysTime = $Process.PrivilegedProcessorTime.TotalMilliseconds
            $userTime = $Process.UserProcessorTime.TotalMilliseconds
            $memoryUsage = $Process.WorkingSet64
            $contextSwitches = $Process.Handles  # Псевдокод, необходимо будет использовать Performance Counters или WMI для точных данных

            # Расчет времени ожидания
            $waitTime = $cpuUsage - $userTime - $sysTime

            # Расчет USER%, SYS%, WAIT%
            $userPercentage = if ($cpuUsage - $userTime -eq 0) { 0 } else { ($userTime / $cpuUsage) * 100 }
            $sysPercentage = if ($cpuUsage - $sysTime -eq 0) { 0 } else { ($sysTime / $cpuUsage) * 100 }
            $waitPercentage = if ($cpuUsage - $waitTime -eq 0) { 0 } else { ($waitTime / $cpuUsage) * 100 }

            # Запись статистики в лог
            $timestamp = (Get-Date).ToString("o")
            "$timestamp, CPU_Usage (ms): $cpuUsage, UserTime (ms): $userTime, SysTime (ms): $sysTime, WaitTime (ms): $waitTime, CPU_User%: $userPercentage, CPU_Sys%: $sysPercentage, CPU_Wait%: $waitPercentage, Memory_Usage (bytes): $memoryUsage, ContextSwitches: $contextSwitches" |
                Out-File -Append -FilePath $LogFile

            # Задержка между сборами статистики
            Start-Sleep -Milliseconds 100
        } catch {
            Write-Host "Error collecting process data for PID $($Process.Id): $_"
            break
        }
    }

    # Время завершения
    $endTime = Get-Date
    $executionTime = $endTime - $startTime
    Write-Host "Total execution time: $($executionTime.TotalSeconds) seconds"
}

# Запуск и сбор метрик для sort.exe программы
if ($SortProgram) {
    Write-Host "Running sort benchmark..."
    try {
        # Запускаем процесс и перенаправляем вывод для диагностики
        $sortProcess = Start-Process -FilePath $SortProgram `
                                      -ArgumentList $sortArguments `
                                      -PassThru `
                                      -RedirectStandardOutput "$OutputLogDir\sort_output.log" `
                                      -RedirectStandardError "$OutputLogDir\sort_error.log"

        # Проверяем, стартовал ли процесс
        if ($sortProcess -and -not $sortProcess.HasExited) {
            Write-Host "Sort program running with PID: $($sortProcess.Id)"
            Collect-ProcessStats -Process $sortProcess -LogFile "$OutputLogDir\sort.log"
            Write-Host "Sort program finished."
        } else {
            Write-Host "Error: Sort program did not start correctly or exited immediately."
            Write-Host "Check $OutputLogDir\sort_error.log for details."
        }
    } catch {
        Write-Host "Exception occurred while starting sort program: $_"
    }
} else {
    Write-Host "Warning: Sort program path is empty. Skipping sort execution."
}

# Запуск и сбор метрик для io-thpt-write программы
if ($IoThptWriteProgram) {
    Write-Host "Running IO throughput write benchmark..."
    try {
        # Запускаем процесс и перенаправляем вывод для диагностики
        $IoThptWriteProcess = Start-Process -FilePath $IoThptWriteProgram `
                                           -ArgumentList $ioArguments `
                                           -PassThru `
                                           -RedirectStandardOutput "$OutputLogDir\io_thpt_write_output.log" `
                                           -RedirectStandardError "$OutputLogDir\io_thpt_write_error.log"

        # Проверяем, стартовал ли процесс
        if ($IoThptWriteProcess -and -not $IoThptWriteProcess.HasExited) {
            Write-Host "IoThptWrite program running with PID: $($IoThptWriteProcess.Id)"
            Collect-ProcessStats -Process $IoThptWriteProcess -LogFile "$OutputLogDir\io_thpt_write.log"
            Write-Host "IoThptWrite program finished."
        } else {
            Write-Host "Error: IoThptWrite program did not start correctly or exited immediately."
            Write-Host "Check $OutputLogDir\io_thpt_write_error.log for details."
        }
    } catch {
        Write-Host "Exception occurred while starting io_thpt_write program: $_"
    }
} else {
    Write-Host "Warning: io_thpt_write program path is empty. Skipping io_thpt_write execution."
}
