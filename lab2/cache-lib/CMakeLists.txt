cmake_minimum_required(VERSION 3.15)
project(cache_library)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к директории с заголовочными файлами
include_directories("include")

add_library(cache_library SHARED src/api.cpp src/lru_policy.cpp)

target_include_directories(cache_library PUBLIC include)

# Добавляем флаг для экспорта функций из DLL
target_compile_definitions(cache_library PRIVATE CACHE_LIBRARY_EXPORTS)

# Добавляем тестовое приложение
add_executable(cache_library_test
        test.cpp
)

# Линкуем тестовое приложение с библиотекой
target_link_libraries(cache_library_test PRIVATE cache_library)

# Указываем тестовому приложению путь к заголовочным файлам
target_include_directories(cache_library_test PRIVATE include)

