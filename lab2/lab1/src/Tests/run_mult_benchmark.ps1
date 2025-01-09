param (
    [string]$OutputLogDir,
    [string]$SortProgram,
    [string]$IoThptWriteProgram,
    [int]$Repetitions = 1,  # Количество повторений
    [string[]]$SortParam,   # Параметры для sort программы
    [string[]]$IoThptWriteParam,  # Параметры для io-thpt-write программы
    [string]$CompilerOptimizationFlag = "/O2"  # Флаг агрессивной оптимизации компилятора
)

# Создание каталога для логов
if (!(Test-Path -Path $OutputLogDir)) {
    Write-Host "Создаем каталог для логов: $OutputLogDir"
    New-Item -ItemType Directory -Path $OutputLogDir | Out-Null
} else {
    Write-Host "Каталог для логов уже существует: $OutputLogDir"
}


# Конвертируем аргументы в строку для передачи программам
$sortArguments = $SortParam -join " "
$ioArguments = $IoThptWriteParam -join " "

function Collect-ProcessStats {
    param (
        [System.Diagnostics.Process]$Process,
        [string]$LogFile
    )

    $startTime = Get-Date
    while (-not $Process.HasExited) {
        try {
            $cpuUsage = $Process.TotalProcessorTime.TotalMilliseconds
            $sysTime = $Process.PrivilegedProcessorTime.TotalMilliseconds
            $userTime = $Process.UserProcessorTime.TotalMilliseconds
            $memoryUsage = $Process.WorkingSet64

            $timestamp = (Get-Date).ToString("o")
            "$timestamp, CPU_Usage (ms): $cpuUsage, UserTime (ms): $userTime, SysTime (ms): $sysTime, Memory_Usage (bytes): $memoryUsage" |
                Out-File -Append -FilePath $LogFile

            Start-Sleep -Milliseconds 100
        } catch {
            Write-Host "Error collecting process data for PID $($Process.Id): $_"
            break
        }
    }

    $endTime = Get-Date
    $executionTime = $endTime - $startTime
    Write-Host "Total execution time: $($executionTime.TotalSeconds) seconds"
}

function Start-Program {
    param (
        [int]$InstanceId,
        [string]$ProgramPath,
        [string]$ProgramArgs,
        [string]$LogDir
    )

    $instanceDir = Join-Path -Path $LogDir -ChildPath "instance_$InstanceId"
    if (!(Test-Path -Path $instanceDir)) {
            Write-Host "Создаем директорию для экземпляра: $instanceDir"
            New-Item -ItemType Directory -Path $instanceDir | Out-Null
    }

    $stdoutLog = Join-Path -Path $instanceDir -ChildPath "stdout.log"
    $stderrLog = Join-Path -Path $instanceDir -ChildPath "stderr.log"
    $statLog = Join-Path -Path $instanceDir -ChildPath "stats.log"

    try {
        Write-Host "Starting instance $InstanceId with Program: $ProgramPath, Args: $ProgramArgs"

        # Запуск процесса
        $process = Start-Process -FilePath $ProgramPath `
                                 -ArgumentList $ProgramArgs `
                                 -PassThru `
                                 -RedirectStandardOutput $stdoutLog `
                                 -RedirectStandardError $stderrLog `
                                 -WorkingDirectory $instanceDir

        # Сбор статистики
        Collect-ProcessStats -Process $process -LogFile $statLog
    } catch {
        Write-Host "Error starting process $InstanceId"
    }
}

# Запуск программ в фоновом режиме с использованием многозадачности
function Run-Benchmark {
    param (
        [string]$ProgramName,
        [int]$InstanceCount,
        [string]$ProgramPath,
        [string]$ProgramArgs,
        [string]$OutputLogDir
    )

    $jobs = @()

    for ($i = 1; $i -le $InstanceCount; $i++) {
        $jobs += Start-Job -ScriptBlock {
            param ($id, $logDir, $program, $arguments)

            Start-Program -InstanceId $id -ProgramPath $program -ProgramArgs $arguments -LogDir $logDir
        } -ArgumentList $i, $OutputLogDir, $ProgramPath, $ProgramArgs
    }

    $jobs | ForEach-Object {
        $_ | Wait-Job | Out-Null
        Write-Host "Job completed: $($_.InstanceId)"
        Remove-Job -Job $_
    }
}

# Запуск SortProgram
if ($SortProgram) {
    Write-Host "Running Sort benchmark..."
    Run-Benchmark -ProgramName "Sort" -InstanceCount $Repetitions -ProgramPath $SortProgram -ProgramArgs $sortArguments -OutputLogDir $OutputLogDir
}

# Запуск IoThptWriteProgram
if ($IoThptWriteProgram) {
    Write-Host "Running IO throughput write benchmark..."
    Run-Benchmark -ProgramName "IoThptWrite" -InstanceCount $Repetitions -ProgramPath $IoThptWriteProgram -ProgramArgs $ioArguments -OutputLogDir $OutputLogDir
}
