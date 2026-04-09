#ifndef __MLB_CFG_H__
#define __MLB_CFG_H__

#ifdef __GNUC__
#define MLB_NOINLINE __attribute__((__noinline__)) 
#define MLB_CALL    __attribute__ ((visibility ("default")))

#elif defined(_WIN32) || defined(__CYGWIN__)

#include <windows.h>
#define MLB_NOINLINE
#define MLB_CALL

#else

#define MLB_NOINLINE
#define MLB_CALL

#endif

#ifndef MLB_INLINE
#define MLB_INLINE
#endif

#define MLB_DEBUG //开启DEBUG日志
#define MLB_TRACE //开启TRACE日志

#include <stdint.h>
#include <stddef.h>

#ifndef _SIZE_T
#ifndef _SIZE_T_DEFINED
typedef uint32_t size_t;
#endif
#endif

#ifndef NULL
#define NULL 0
#endif




#ifdef __cplusplus
    #ifndef EXTERN_C
    #define EXTERN_C       extern "C"
    #endif
    
#else
    #ifndef EXTERN_C
    #define EXTERN_C       extern
    #endif
#endif


#endif
