#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <string.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define LCD_WIDTH 128
#define LCD_HEIGHT 64
enum DrawDir {
    DRAW_NWSE,
    DRAW_SWNE,
};

void _msleep(unsigned t);
#endif //_COMMON_H_
