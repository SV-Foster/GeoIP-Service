/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once
#ifdef __cplusplus
extern "C" {
#endif


#define PRINTF_BINARY_PATTERN_INT8 TEXT("%c%c%c%c%c%c%c%c")
#define PRINTF_BYTE_TO_BINARY_INT8(i)    \
    (((i) & 0x80ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x40ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x20ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x10ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x08ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x04ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x02ll) ? TEXT('1') : TEXT('0')), \
    (((i) & 0x01ll) ? TEXT('1') : TEXT('0'))

#define PRINTF_BINARY_PATTERN_INT16 \
    PRINTF_BINARY_PATTERN_INT8              PRINTF_BINARY_PATTERN_INT8
#define PRINTF_BYTE_TO_BINARY_INT16(i) \
    PRINTF_BYTE_TO_BINARY_INT8((i) >> 8),   PRINTF_BYTE_TO_BINARY_INT8(i)
#define PRINTF_BINARY_PATTERN_INT32 \
    PRINTF_BINARY_PATTERN_INT16             PRINTF_BINARY_PATTERN_INT16
#define PRINTF_BYTE_TO_BINARY_INT32(i) \
    PRINTF_BYTE_TO_BINARY_INT16((i) >> 16), PRINTF_BYTE_TO_BINARY_INT16(i)
#define PRINTF_BINARY_PATTERN_INT64    \
    PRINTF_BINARY_PATTERN_INT32             PRINTF_BINARY_PATTERN_INT32
#define PRINTF_BYTE_TO_BINARY_INT64(i) \
    PRINTF_BYTE_TO_BINARY_INT32((i) >> 32), PRINTF_BYTE_TO_BINARY_INT32(i)

#ifdef __cplusplus
    }
#endif
