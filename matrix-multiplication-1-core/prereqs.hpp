#ifndef PREREQS_HPP
#define PREREQS_HPP

#include <stdint.h>

typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;

typedef float       f32;
typedef double      f64;
typedef long double f80;

enum UserInterrupt {
	NotDone = 0x0,
	Done    = 0x1,
};

#endif // PREREQS_HPP
