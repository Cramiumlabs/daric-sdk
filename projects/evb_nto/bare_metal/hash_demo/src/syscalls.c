/**
 *******************************************************************************
 * @file    syscalls.c
 * @author  Daric Team
 * @brief   Source file for syscalls.c module.
 *******************************************************************************
 * @attention
 *
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */
#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include "system_daric.h"

/**
 * These are dummy implementations of syscalls required by the
 * C library when building for bare-metal targets.
 */

#if defined(__ARMCC_VERSION)
/* ArmClang (MDK-ARM) implementations using $Sub$$/$Super$$ wrapping if needed,
   or direct overrides of the ARM C Library functions. */

/* Disable semihosting */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
#else
#pragma import(__use_no_semihosting)
#endif

int fputc(int ch, FILE *f)
{
    (void)f;
    if (ch == '\n')
    {
        DUART_PutChar('\r');
    }
    DUART_PutChar(ch);
    return ch;
}

int fgetc(FILE *f)
{
    (void)f;
    return 0;
}

int ferror(FILE *f)
{
    (void)f;
    return 0;
}

void _sys_exit(int x)
{
    (void)x;
    while (1)
        ;
}

/* System call overrides to prevent semihosting */
int $Sub$$_sys_open(const char *name, int x)
{
    (void)name;
    (void)x;
    return 0;
}

int $Sub$$_sys_istty(int fh)
{
    (void)fh;
    return 0;
}

int $Sub$$_sys_read(int fh, unsigned char *buf, unsigned len, int mode)
{
    (void)fh;
    (void)buf;
    (void)len;
    (void)mode;
    return 0;
}

#elif defined(__GNUC__)
/* Newlib (GCC) implementations */
#include <reent.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char __HeapBase;  /* Defined by the linker */
extern char __HeapLimit; /* Defined by the linker */

void _sys_exit(int x)
{
    (void)x;
    while (1)
        ;
}

int _close_r(struct _reent *r, int fd)
{
    (void)fd;
    r->_errno = ENOSYS;
    return -1;
}

int _fstat_r(struct _reent *r, int fd, struct stat *st)
{
    (void)fd;
    (void)st;
    r->_errno = ENOSYS;
    return -1;
}

int _isatty_r(struct _reent *r, int fd)
{
    (void)fd;
    r->_errno = ENOSYS;
    return -1;
}

_off_t _lseek_r(struct _reent *r, int fd, _off_t ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
    r->_errno = ENOSYS;
    return (_off_t)-1;
}

_ssize_t _read_r(struct _reent *r, int fd, void *buf, size_t len)
{
    (void)fd;
    (void)buf;
    (void)len;
    r->_errno = ENOSYS;
    return (_ssize_t)-1;
}

_ssize_t _write_r(struct _reent *r, int fd, const void *buf, size_t len)
{
    size_t      i;
    const char *data = (const char *)buf;

    (void)r;
    (void)fd;

    for (i = 0; i < len; i++)
    {
        if (data[i] == '\n')
        {
            DUART_PutChar('\r');
        }
        DUART_PutChar(data[i]);
    }
    return (_ssize_t)len;
}

int _getpid_r(struct _reent *r)
{
    (void)r;
    return 1;
}

int _kill_r(struct _reent *r, int pid, int sig)
{
    (void)pid;
    (void)sig;
    r->_errno = EINVAL;
    return -1;
}

/**
 * _sbrk - Increase program data space.
 * Malloc and related functions depend on this.
 */
void *_sbrk_r(struct _reent *r, ptrdiff_t incr)
{
    static char *heap_ptr = NULL;
    char        *prev_heap_ptr;

    if (heap_ptr == NULL)
    {
        heap_ptr = &__HeapBase;
    }

    prev_heap_ptr = heap_ptr;

    if (heap_ptr + incr > &__HeapLimit)
    {
        /* Heap overflow */
        r->_errno = ENOMEM;
        return (void *)-1;
    }

    heap_ptr += incr;

    return (void *)prev_heap_ptr;
}

#endif

/* Common implementations */
void _ttywrch(int ch)
{
    (void)ch;
}