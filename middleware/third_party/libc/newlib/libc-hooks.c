/**
 ******************************************************************************
 * @file    libc-hooks.c
 * @author  System Team
 * @brief   This file provides a self implementation hooks of newlib's functions.
 ******************************************************************************
 * @attention
 *
 * © Copyright CrossBar, Inc. 2024.

 * All rights reserved.

 * This software is the proprietary property of CrossBar, Inc. and is protected
 * by copyright laws. Any unauthorized reproduction, distribution, or
 * modification is strictly prohibited.
 *
 ******************************************************************************
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "tx_api.h"

#if ((__NEWLIB__ == 2) && (__NEWLIB_MINOR__ < 5)) || \
    ((__NEWLIB__ == 3) && (__NEWLIB_MINOR__ > 1))
#warning \
    "This wrapper was verified for newlib versions 2.5 - 3.1; please ensure newlib's external requirements for malloc-family are unchanged!"
#endif

#ifndef ARG_UNUSED
#define ARG_UNUSED(x) (void)(x)
#endif

#ifndef __weak
#define __weak __attribute__((__weak__))
#endif

#ifndef FUNC_ALIAS
#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, return_type) \
    return_type new_alias() ALIAS_OF(real_func)
#endif

/* Memory Manage related functions */

extern TX_BYTE_POOL DefaultHeap;

typedef struct alloc_info
{
    void *ptr;
    size_t size;
    #ifdef CONFIG_DUMP_HEAPINFO
    void *pAddr;
    void *dummy;
    #endif
} alloc_info_t;

void *aligned_alloc(size_t alignment, size_t size)
{
    void *user_ptr, *ptr = NULL;

    if (size == 0)
    {
        return NULL;
    }

    if (((alignment & (alignment - 1)) != 0) ||
        (alignment % sizeof(void *) != 0))
    {
        return NULL;
    }

    tx_byte_allocate(&DefaultHeap, &ptr, sizeof(alloc_info_t) + alignment + size,
                     TX_NO_WAIT);

    if (ptr != NULL)
    {
        user_ptr = (void *)((uintptr_t)ptr + sizeof(alloc_info_t));

        if (alignment != 0)
        {
            user_ptr = (void *)((uintptr_t)user_ptr + alignment -
                                (uintptr_t)user_ptr % alignment);
        }

        alloc_info_t *info =
            (alloc_info_t *)((uintptr_t)user_ptr - sizeof(alloc_info_t));
        info->ptr = ptr;
        info->size = size;

        ptr = user_ptr;
    }

    return ptr;
}

void *malloc(size_t size)
{
    void *rtnAddr;
    #ifdef CONFIG_DUMP_HEAPINFO
    alloc_info_t *pAllocInfo;
    void *pAddr;
    __asm volatile("mov %0, lr" : "=r"(pAddr));
    (void)(pAddr);
    #endif

    rtnAddr = aligned_alloc((size_t)0, size);
    #ifdef CONFIG_DUMP_HEAPINFO
    pAllocInfo = rtnAddr - sizeof(alloc_info_t);
    if ((pAddr >= (void *)_malloc_r && pAddr < ((void *)_malloc_r + 0x1a))
       || (pAddr >= (void *)calloc && pAddr < ((void *)calloc + 0x4e))
       || (pAddr >= (void *)realloc && pAddr < ((void *)realloc + 0x8c))
       )
    {
        pAllocInfo->pAddr = 0;
    }
    else
    {
        pAllocInfo->pAddr = pAddr;
    }
    #endif
    return rtnAddr;
}

#ifdef CONFIG_DUMP_HEAPINFO
/**
 * @brief Get the amount of free memory in the default heap pool.
 * @return uint32_t The number of free bytes currently available in the default heap.
 * 
 * @note The function only retrieves the free size information.
 *       other pool attributes are ignored by passing NULL pointers for those parameters.
 *       Total free memory may be fragmented.
 *
 * This function queries the memory pool information to retrieve the current
 * amount of free memory available for allocation in the default heap pool.
 */
uint32_t getHeapFreeSize()
{
    uint32_t freeSize;
    tx_byte_pool_info_get(&DefaultHeap, NULL, &freeSize, NULL, NULL, NULL, NULL);
    return freeSize;
}

/**
 * @brief Traverse and dump detailed information about allocated blocks in the default heap.
 * @return none
 *
 * @note This function disables interrupts during traversal to ensure heap consistency
 *       while inspecting internal structures. Interrupts are restored before returning.
 *
 * This function performs a low-level traversal of the internal block structure of the
 * default heap memory pool. It walks through the heap's block list, 
 * identifies allocated blocks, and prints detailed information for each allocated block.
 * The function also calculates and prints summary statistics 
 * including total allocation count, 
 * total allocated bytes, and remaining free heap space.
 */
void sysDumpHeapInfo()
{
    uint8_t *pCurBlock;
    uint8_t *pHeapEnd;
    uint32_t allocCount = 0;
    uint32_t totalAllocSize = 0;
    TX_BYTE_POOL *pHeapPool = &DefaultHeap;

    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE

    printf("Traverse heap start!\r\n");
    pCurBlock = pHeapPool->tx_byte_pool_start;
    pHeapEnd = pHeapPool->tx_byte_pool_start + pHeapPool->tx_byte_pool_size;

    while (pCurBlock < pHeapEnd)
    {
        uint8_t **pBlkHead = (uint8_t **)pCurBlock;
        uint8_t *next_block = pBlkHead[0];
        uint8_t *owner_info = pBlkHead[1];

        uint32_t block_size;
        if (next_block != NULL && next_block > pCurBlock)
        {
            block_size = (uint32_t)(next_block - pCurBlock) - sizeof(uint8_t *) * 2;
        }
        else
        {
            break;
        }

        if (owner_info == (uint8_t *)pHeapPool)
        {
            allocCount++;
            totalAllocSize += block_size;
            alloc_info_t *pData = (alloc_info_t *)(pCurBlock + sizeof(uint8_t *) * 2);
            if (pData->pAddr)
            {
                printf("%p, %d\r\n", pData->pAddr, pData->size);
            }
        }

        pCurBlock = next_block;

        if (pCurBlock == NULL || pCurBlock >= pHeapEnd)
        {
            break;
        }
    }

    printf("Traverse heap Completed!\r\n");
    printf("Total alloc count: %lu \r\n", allocCount);
    printf("Total alloc size: %lu \r\n", totalAllocSize);
    printf("Heap free size: %lu \r\n", pHeapPool->tx_byte_pool_available);

    TX_RESTORE
}
#endif

void *_malloc_r(struct _reent *ptr, size_t size)
{
    return malloc(size);
}

void free(void *ptr)
{
    if (ptr != NULL)
    {
        alloc_info_t *info =
            (alloc_info_t *)((uintptr_t)ptr - sizeof(alloc_info_t));
        tx_byte_release(info->ptr);
    }
}
void _free_r(struct _reent *r, void *p)
{
    free(p);
}

void *calloc(size_t nitems, size_t size)
{
    #ifdef CONFIG_DUMP_HEAPINFO
    alloc_info_t *pAllocInfo;
    void *pAddr;
    __asm volatile("mov %0, lr" : "=r"(pAddr));
    #endif

    void *ptr = malloc(nitems * size);
    if (ptr != NULL)
    {
        memset(ptr, 0, nitems * size);
    }

    #ifdef CONFIG_DUMP_HEAPINFO
    pAllocInfo = ptr - sizeof(alloc_info_t);
    pAllocInfo->pAddr = pAddr;
    #endif

    return ptr;
}
void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
    return calloc(size, len);
}

void *realloc(void *ptr, size_t size)
{
    #ifdef CONFIG_DUMP_HEAPINFO
    alloc_info_t *pAllocInfo;
    void *pAddr;
    __asm volatile("mov %0, lr" : "=r"(pAddr));
    #endif

    void *pMallocAddr;
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    if (ptr == NULL)
    {
        pMallocAddr = malloc(size);
        #ifdef CONFIG_DUMP_HEAPINFO
        pAllocInfo = pMallocAddr - sizeof(alloc_info_t);
        pAllocInfo->pAddr = pAddr;
        #endif

        return pMallocAddr;
    }

    char *new_ptr = (char *)malloc(size);
    if (new_ptr == NULL)
    {
        return NULL;
    }

    alloc_info_t *info = (alloc_info_t *)((uintptr_t)ptr - sizeof(alloc_info_t));
    size_t copy_size = (size < info->size) ? size : info->size;
    memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    #ifdef CONFIG_DUMP_HEAPINFO
    pAllocInfo = (void *)new_ptr - sizeof(alloc_info_t);
    pAllocInfo->pAddr = pAddr;
    #endif

    return new_ptr;
}
void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    return realloc(old, newlen);
}

/*
 * These are stubs for the standard I/O functions. They are used by the
 * standard library to write to stdout and read from stdin.
 */

static int _stdout_hook_default(int c)
{
    (void)(c); /* Prevent warning about unused argument */

    return EOF;
}

static int (*_stdout_hook)(int) = _stdout_hook_default;

void __stdout_hook_install(int (*hook)(int)) { _stdout_hook = hook; }

static unsigned char _stdin_hook_default(void) { return 0; }

static unsigned char (*_stdin_hook)(void) = _stdin_hook_default;

void __stdin_hook_install(unsigned char (*hook)(void)) { _stdin_hook = hook; }

#ifndef CONFIG_POSIX_API
static int __read_stdin(char *buf, int nbytes)
{
    int i = 0;

    for (i = 0; i < nbytes; i++)
    {
        *(buf + i) = _stdin_hook();
        if ((*(buf + i) == '\n') || (*(buf + i) == '\r'))
        {
            i++;
            break;
        }
    }
    return i;
}

__weak int _read(int fd, char *buf, int nbytes)
{
    ARG_UNUSED(fd);

    return __read_stdin(buf, nbytes);
}
__weak FUNC_ALIAS(_read, read, int);

static int __write_stdout(const void *buffer, int nbytes)
{
    const char *buf = buffer;
    int i;

    for (i = 0; i < nbytes; i++)
    {
        if (*(buf + i) == '\n')
        {
            _stdout_hook('\r');
        }
        _stdout_hook(*(buf + i));
    }
    return nbytes;
}

__weak int _write(int fd, const void *buf, int nbytes)
{
    ARG_UNUSED(fd);

    return __write_stdout(buf, nbytes);
}
__weak FUNC_ALIAS(_write, write, int);

__weak int _open(const char *name, int mode) { return -1; }
__weak FUNC_ALIAS(_open, open, int);

__weak int _close(int file) { return -1; }
__weak FUNC_ALIAS(_close, close, int);

__weak int _lseek(int file, int ptr, int dir) { return 0; }
__weak FUNC_ALIAS(_lseek, lseek, int);
#else
extern ssize_t write(int file, const char *buffer, size_t count);
#define _write write
#endif

__weak int _isatty(int file) { return file <= 2; }
__weak FUNC_ALIAS(_isatty, isatty, int);

__weak int _kill(int i, int j) { return 0; }
__weak FUNC_ALIAS(_kill, kill, int);

__weak int _getpid(void) { return 0; }
__weak FUNC_ALIAS(_getpid, getpid, int);

__weak int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}
__weak FUNC_ALIAS(_fstat, fstat, int);

__weak void _exit(int status)
{
    _write(1, "exit\n", 5);
    while (1)
    {
    }
}

__weak void abort(void)
{
    _write(1, "abort\n", 5);
    while (1)
    {
    }
}

void _fini(void) {}
