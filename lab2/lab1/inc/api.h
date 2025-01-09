#ifndef API_H
#define API_H

#include <cstddef>
#include <vector>

#ifdef _WIN32
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif

#ifdef CACHE_LIBRARY_EXPORTS
#define CACHE_API __declspec(dllexport)
#else
#define CACHE_API __declspec(dllimport)
#endif


// Define the file descriptor type
typedef int lab2_fd;

// Public API declarations
extern "C" {
    CACHE_API lab2_fd lab2_open(const char* path);
    CACHE_API int lab2_close(lab2_fd fd);
    CACHE_API ssize_t lab2_read(lab2_fd fd, void* buf, size_t count);
    CACHE_API ssize_t lab2_write(lab2_fd fd, const void* buf, size_t count);
    CACHE_API off_t lab2_lseek(lab2_fd fd, off_t offset, int whence);
    CACHE_API int lab2_fsync(lab2_fd fd);
    CACHE_API void lab2_set_cache_enabled(bool enabled);
}

#endif API_H