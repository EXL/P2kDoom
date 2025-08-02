#ifndef STDINT_H
#define STDINT_H

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

typedef unsigned int       size_t;

#define int_fast8_t register signed char
#define uint_fast8_t register unsigned char

#define INT8_MIN       (-128)
#define INT16_MIN      (-32767-1)
#define INT32_MIN      (-2147483647-1)
#define INT64_MIN      (-9223372036854775807LL-1)

#define INT8_MAX       (127)
#define INT16_MAX      (32767)
#define INT32_MAX      (2147483647)
#define INT64_MAX      (9223372036854775807LL)

#define UINT8_MAX      (255)
#define UINT16_MAX     (65535)
#define UINT32_MAX     (4294967295U)
#define UINT64_MAX     (18446744073709551615ULL)

#define printf PFprintf
#define exit(x)

#define labs(i) ((i) < 0 ? -(i) : (i))

#define I_Error PFprintf

#endif // STDINT_H
