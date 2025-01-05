#ifndef CACHE_LIBRARY_API_H
#define CACHE_LIBRARY_API_H

#include <cstddef>
#include <vector>

#ifdef _WIN32
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif

// Define the file descriptor type
typedef int lab2_fd;

// Public API declarations
extern "C" {
    lab2_fd lab2_open(const char *path);
    int lab2_close(lab2_fd fd);
    ssize_t lab2_read(lab2_fd fd, void *buf, size_t count);
    ssize_t lab2_write(lab2_fd fd, const void *buf, size_t count);
    off_t lab2_lseek(lab2_fd fd, off_t offset, int whence);
    int lab2_fsync(lab2_fd fd);
}

#endif // CACHE_LIBRARY_API_H