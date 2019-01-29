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

void USB_Enable(unsigned use_interrupt);
void USB_Disable();
/* SPI Flash */
void SPIFlash_EraseSector(u32 sectorAddress);
void SPIFlash_WriteBytes(u32 writeAddress, u32 length, const u8 * buffer);
void SPIFlash_ReadBytes(u32 readAddress, u32 length, u8 * buffer);

#define SPIFLASH_SECTORS 512
#define SPIFLASH_SECTOR_OFFSET 0

#endif //_COMMON_H_
