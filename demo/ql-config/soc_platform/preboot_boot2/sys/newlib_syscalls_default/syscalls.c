#include <unistd.h>
#include <reent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdint.h>

#include "dbg.h"

/**
 * @brief manage the heap
 */
extern char __heap_start;               /* start of the heap */
extern char __heap_end;                 /* end of the heap */
static char *heap_top = &__heap_start + 4;

/**
 * @brief Exit a program without cleaning up files
 *
 * If your system doesn't provide this, it is best to avoid linking with subroutines that
 * require it (exit, system).
 *
 * @param n     the exit code, 0 for all OK, >0 for not OK
 */
void _exit(int n)
{
    (void) n;
    while (1) ;
}

/**
 * @brief Allocate memory from the heap.
 *
 * The current heap implementation is very rudimentary, it is only able to allocate
 * memory. But it does not have any means to free memory again
 *
 * @return      pointer to the newly allocated memory on success
 * @return      pointer set to address `-1` on failure
 */
void *_sbrk_r(struct _reent *r, ptrdiff_t incr)
{
    void *res = heap_top;

    if ((heap_top + incr > &__heap_end) || (heap_top + incr < &__heap_start)) {
        r->_errno = ENOMEM;
        res = (void *)-1;
    }
    else {
        heap_top += incr;
    }

    return res;
}

/**
 * @brief Get the process-ID of the current thread
 *
 * @return      the process ID of the current thread
 */
pid_t _getpid(void)
{
    return 0;
}

/**
 * @brief Get the process-ID of the current thread
 *
 * @return      the process ID of the current thread
 */
pid_t _getpid_r(struct _reent *ptr)
{
    (void) ptr;
    return 0;
}

/**
 * @brief Send a signal to a given thread
 *
 * @param r     TODO
 * @param pid   TODO
 * @param sig   TODO
 *
 * @return      TODO
 */
__attribute__ ((weak))
int _kill_r(struct _reent *r, pid_t pid, int sig)
{
    (void) pid;
    (void) sig;
    r->_errno = ESRCH;                      /* not implemented yet */
    return -1;
}

/*
 * Fallback read function
 *
 * All input is read from uart_stdio regardless of fd number. The function will
 * block until a byte is actually read.
 *
 * Note: the read function does not buffer - data will be lost if the function is not
 * called fast enough.
 */
_ssize_t _read_r(struct _reent *r, int fd, void *buffer, size_t count)
{
    (void) r;
    (void) fd;
    (void) buffer;
    (void) count;
    return 0;
}

/*
 * Fallback write function
 *
 * All output is directed to uart_stdio, independent of the given file descriptor.
 * The write call will further block until the byte is actually written to the UART.
 */
_ssize_t _write_r(struct _reent *r, int fd, const void *data, size_t count)
{
    (void) r;
    (void) fd;
    return dbg_send_bytes(data, count);
}

/* Stubs to avoid linking errors, these functions do not have any effect */
int _open_r(struct _reent *r, const char *name, int flags, int mode)
{
    (void) name;
    (void) flags;
    (void) mode;
    r->_errno = ENODEV;
    return -1;
}

int _close_r(struct _reent *r, int fd)
{
    (void) fd;
    r->_errno = ENODEV;
    return -1;
}

_off_t _lseek_r(struct _reent *r, int fd, _off_t pos, int dir)
{
    (void) fd;
    (void) pos;
    (void) dir;
    r->_errno = ENODEV;
    return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
    (void) fd;
    (void) st;
    r->_errno = ENODEV;
    return -1;
}

int _stat_r(struct _reent *r, const char *name, struct stat *st)
{
    (void) name;
    (void) st;
    r->_errno = ENODEV;
    return -1;
}

int _unlink_r(struct _reent *r, const char *path)
{
    (void) path;
    r->_errno = ENODEV;
    return -1;
}

/**
 * @brief Query whether output stream is a terminal
 *
 * @param r     TODO
 * @param fd    TODO
 *
 * @return      TODO
 */
int _isatty_r(struct _reent *r, int fd)
{
    r->_errno = 0;

    if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        return 1;
    }

    return 0;
}

/**
 * @brief Send a signal to a thread
 *
 * @param[in] pid the pid to send to
 * @param[in] sig the signal to send
 *
 * @return TODO
 */
__attribute__ ((weak))
int _kill(pid_t pid, int sig)
{
    (void) pid;
    (void) sig;
    errno = ESRCH;                         /* not implemented yet */
    return -1;
}

int _gettimeofday_r(struct _reent *r, struct timeval *restrict tp, void *restrict tzp)
{
    (void) r;
    (void) tp;
    (void) tp;
    (void) tzp;
    r->_errno = EINVAL;
    return -1;
}
