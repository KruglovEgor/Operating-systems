param (
    [int]$NumberOfRuns = 4,  # Количество запусков
    [string]$SortProgramPath = "E:\Program Files\CLionProjects\osi_1\cmake-build-release\sort.exe",
    [string]$IoThptWriteProgramPath = "E:\Program Files\CLionProjects\osi_1\cmake-build-release\io-thpt-write.exe",
    [string]$CompilerOptimizationFlag = "/O2"  # Оптимизация для компилятора
)

# Параметры для программ
$SortArguments = @("100000000")
$IoThptWriteArguments = @("output.dat", "100000", "10000")


# Проверка путей программ
if (!(Test-Path $SortProgramPath)) {
    Write-Host "Ошибка: Путь к программе Sort неверный: $SortProgramPath"
    return
}

if (!(Test-Path $IoThptWriteProgramPath)) {
    Write-Host "Ошибка: Путь к программе IoThptWrite неверный: $IoThptWriteProgramPath"
    return
}

# Запуск SortProgram
for ($i = 0; $i -lt $NumberOfRuns; $i++) {
    Write-Host "Запуск экземпляра Sort - $($i + 1)"
    .\run_mult_benchmark.ps1 -OutputLogDir "sort_benchmark" -SortProgram $SortProgramPath -SortParam $SortArguments -Repetitions 1 -CompilerOptimizationFlag $CompilerOptimizationFlag
}

# Запуск IoThptWriteProgram
for ($i = 0; $i -lt $NumberOfRuns; $i++) {
    Write-Host "Запуск экземпляра io-thpt-write - $($i + 1)"
    .\run_mult_benchmark.ps1 -OutputLogDir "io_benchmark" -IoThptWriteProgram $IoThptWriteProgramPath -IoThptWriteParam $IoThptWriteArguments -Repetitions 1 -CompilerOptimizationFlag $CompilerOptimizationFlag
}

# Запуск обоих программ одновременно
for ($i = 0; $i -lt $NumberOfRuns; $i++) {
    Write-Host "Запуск обоих программ (Sort и IoThptWrite) - $($i + 1)"
    .\run_mult_benchmark.ps1 -OutputLogDir "combined_benchmark" -SortProgram $SortProgramPath -SortParam $SortArguments -IoThptWriteProgram $IoThptWriteProgramPath -IoThptWriteParam $IoThptWriteArguments -Repetitions 1 -CompilerOptimizationFlag $CompilerOptimizationFlag
}
