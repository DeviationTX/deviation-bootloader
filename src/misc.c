#include "common.h"

void _msleep(unsigned t) {
    volatile unsigned i = 0x8000 * t;
    while(i) i--;
}

