cmake_minimum_required(VERSION 3.20)
project(osi_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/EHsc)

add_executable(osi_1 src/Main.cpp
        src/CommandExecutor.cpp
        src/CommandExecutor.h
        src/Shell.cpp
        src/Shell.h
        src/Commands/CatCommand.h
        src/Commands/HistoryCommand.h
        src/Commands/MkdirCommand.h
        src/Commands/HelpCommand.h
        src/Commands/RmCommand.h
        src/Commands/TouchCommand.h)

add_executable(io-thpt-write src/Benchmarks/IoThptWrite.cpp)
add_executable(sort src/Benchmarks/Sort.cpp)
add_executable(combined src/Benchmarks/Comdined.cpp)

# Подключение библиотеки ntdll
target_link_libraries(osi_1 PRIVATE ntdll.lib)

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /arch:AVX2")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -funroll-loops")
endif()

add_executable(test-proc src/TestDir/TestProc.cpp)
target_link_libraries(test-proc PRIVATE pdh.lib)

# Указываем путь к заголовкам
target_include_directories(io-thpt-write PRIVATE inc)
target_include_directories(combined PRIVATE inc)


# Линкуем с библиотекой
target_link_directories(io-thpt-write PRIVATE lib)
target_link_libraries(io-thpt-write PRIVATE cache_library)
target_link_directories(combined PRIVATE lib)
target_link_libraries(combined PRIVATE cache_library)