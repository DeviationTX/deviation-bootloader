/*
    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Deviation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/cortex.h>
#include "hardware.h"

#ifndef HAS_FLASH_DETECT
    #define HAS_FLASH_DETECT 1
#endif

uint32_t spiflash_sectors     = 0;
#if HAS_FLASH_DETECT
// Defaults for SST25VFxxxB, Devo 10 original memory
static uint8_t SPIFLASH_SR_ENABLE    = 0x50;
static uint8_t SPIFLASH_PROTECT_MASK = 0x3C;
static uint8_t SPIFLASH_WRITE_SIZE   = 2;
static uint8_t SPIFLASH_WRITE_CMD    = 0xAD;
static uint8_t SPIFLASH_FAST_READ    = 0;
static uint8_t SPIFLASH_USE_AAI      = 1;
#else
    #include "spi_flash.h"
#endif

uint32_t SPIFlash_ReadID();
void SPIFlash_WriteBytes(uint32_t writeAddress, uint32_t length, const uint8_t * buffer);
void SPIFlash_WriteByte(uint32_t writeAddress, const unsigned byte);

static void CS_HI()
{
    PORT_pin_set(FLASH_CSN_PIN);
}

static void CS_LO()
{
    PORT_pin_clear(FLASH_CSN_PIN);
}

#if HAS_FLASH_DETECT
/*
 * Detect flash memory type and set variables controlling
 * access accordingly.
 */
void detect_memory_type()
{
    /* When we have an amount of 4096-byte sectors, fill out Structure for file
     * system
     */
    uint8_t mfg_id, memtype, capacity;
    uint32_t id;
    CS_LO();
    spi_xfer(SPIx, 0x9F);
    mfg_id  = (uint8_t)spi_xfer(SPIx, 0);
    memtype = (uint8_t)spi_xfer(SPIx, 0);
    capacity = (uint8_t)spi_xfer(SPIx, 0);
    CS_HI();
    switch (mfg_id) {
    case 0xBF: // Microchip
        if (memtype == 0x25) {
            spiflash_sectors = 1 << ((capacity & 0x07) + 8);
        }
        if (memtype == 0x26) {
            SPIFLASH_SR_ENABLE    = 0x06;  //No EWSR, use standard WREN
            SPIFLASH_PROTECT_MASK = 0;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors = 1 << ((capacity & 0x07) + 8);
        }
        break;
    case 0xEF: // Winbond
        if (memtype == 0x40) {
            SPIFLASH_PROTECT_MASK = 0x1C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors      = 1 << ((capacity & 0x0f) + 4);
        }
        break;
    case 0x7F: // Extension code, older ISSI, maybe some others
        if (memtype == 0x9D && capacity == 0x46) {
            SPIFLASH_SR_ENABLE    = 0x06;  //No EWSR, use standard WREN
            SPIFLASH_PROTECT_MASK = 0x3C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors      = 1024;
        }
        break;
    case 0x9D: // ISSI
        if (memtype == 0x60 || memtype == 0x40 || memtype == 0x30) {
            SPIFLASH_SR_ENABLE    = 0x06;  //No EWSR, use standard WREN
            SPIFLASH_PROTECT_MASK = 0x3C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors      = 1 << ((capacity & 0x0f) + 4);
        }
        break;
    case 0xC2: // Macronix
        if (memtype == 0x20) {
            SPIFLASH_SR_ENABLE    = 0x06;  //No EWSR, use standard WREN
            SPIFLASH_PROTECT_MASK = 0x3C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors      = 1 << ((capacity & 0x0f) + 4);
        }
        break;
    case 0x1F: // Adesto
        if ((memtype & 0xE0) == 0x40) {
            SPIFLASH_SR_ENABLE    = 0x06;  //No EWSR, use standard WREN
            SPIFLASH_PROTECT_MASK = 0x3C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0x02;
            SPIFLASH_FAST_READ    = 1;
            SPIFLASH_USE_AAI      = 0;
            spiflash_sectors      = 1 << ((memtype & 0x1f) + 3);
        }
        break;
    default:
        /* Check older READ ID command */
        id = SPIFlash_ReadID();
        if (id == 0xBF48BF48) {
            SPIFLASH_PROTECT_MASK = 0x0C;
            SPIFLASH_WRITE_SIZE   = 1;
            SPIFLASH_WRITE_CMD    = 0xAF;
            spiflash_sectors      = 16;
        }
        break;
    }
}
#endif
/*
 *
 */
void SPIFlash_Init()
{
    /* Enable SPIx */
    rcc_periph_clock_enable(RCC_GPIOB);  // Assume sck, mosi, miso all on same port
    PORT_mode_setup(FLASH_CSN_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    CS_HI();
#if HAS_FLASH_DETECT
    detect_memory_type();
#endif
}
/*
 *
 */
static void SPIFlash_SetAddr(unsigned cmd, uint32_t address)
{
    CS_LO();
    spi_xfer(SPIx, cmd);
    spi_xfer(SPIx, (uint8_t)(address >> 16));
    spi_xfer(SPIx, (uint8_t)(address >>  8));
    spi_xfer(SPIx, (uint8_t)(address));
}
/*
 *
 */
uint32_t SPIFlash_ReadID()
{
    uint32_t result;

    SPIFlash_SetAddr(0x90, 0);
    result  = (uint8_t)spi_xfer(SPIx, 0);
    result <<= 8;
    result |= (uint8_t)spi_xfer(SPIx, 0);
    result <<= 8;
    result |= (uint8_t)spi_xfer(SPIx, 0);
    result <<= 8;
    result |= (uint8_t)spi_xfer(SPIx, 0);
   
    CS_HI();
    return result;
}
/*
 *
 */
void WriteFlashWriteEnable()
{
    CS_LO();
    spi_xfer(SPIx, 0x06);
    CS_HI();
}
/*
 *
 */
void WriteFlashWriteDisable()
{
    CS_LO();
    spi_xfer(SPIx, 0x04);
    CS_HI();
}
/*
 *
 */
void WaitForWriteComplete()
{
    unsigned sr;
    // We disable interrupts in SPI operation so we
    // need to periodically re-enable them.
    while(true) {
        int i;
        CS_LO();
        spi_xfer(SPIx, 0x05);
        for (i = 0; i < 100; ++i) {
            sr = spi_xfer(SPIx, 0x00);
            if (!(sr & 0x01)) break;
        }
        CS_HI();
        if (i < 100) break;
    }
}
/*
 *
 */
void SPIFlash_BlockWriteEnable(unsigned enable)
{
    CS_LO();
    spi_xfer(SPIx, SPIFLASH_SR_ENABLE);
    CS_HI();
    if (SPIFLASH_PROTECT_MASK) {
        CS_LO();
        spi_xfer(SPIx, 0x01);
        spi_xfer(SPIx, enable ? 0 : SPIFLASH_PROTECT_MASK);
        CS_HI();
    } else {
        //Microchip SST26VFxxxB Serial Flash
        if(enable) {
            CS_LO();
            //Global Block Protection unlock
            spi_xfer(SPIx, 0x98);
            CS_HI();
        } else {
            CS_LO();
            //Write Block-Protection Register
            spi_xfer(SPIx, 0x42);
            //write-protected BPR[79:0] = 5555 FFFFFFFF FFFFFFFF
            //data input must be most significant bit(s) first
            spi_xfer(SPIx, 0x55);
            spi_xfer(SPIx, 0x55);
            for (int i = 0; i < 8; i++) {
                spi_xfer(SPIx, 0xFF);
            }
            CS_HI();
            WaitForWriteComplete();
        }
    }
    WriteFlashWriteDisable();
}
/*
 *
 */
static void DisableHWRYBY()
{
    CS_LO();
    spi_xfer(SPIx, 0x80);
    CS_HI();
}
/*
 *
 */
void SPIFlash_EraseSector(uint32_t sectorAddress)
{
    WriteFlashWriteEnable();

    SPIFlash_SetAddr(0x20, sectorAddress);
    CS_HI();

    WaitForWriteComplete();
    WriteFlashWriteDisable();
}
/*
 *
 */
void SPIFlash_BulkErase()
{
    WriteFlashWriteEnable();

    CS_LO();
    spi_xfer(SPIx, 0xC7);
    CS_HI();

    WaitForWriteComplete();
    WriteFlashWriteDisable();
}
/*
 *
 */
void SPIFlash_WriteBytes(uint32_t writeAddress, uint32_t length, const uint8_t * buffer)
{
    uint32_t i = 0;
    if(!length) return; // just in case...

    if (SPIFLASH_USE_AAI)
        DisableHWRYBY();

    WriteFlashWriteEnable();

    if (SPIFLASH_USE_AAI) {
        if (SPIFLASH_WRITE_SIZE == 2 && writeAddress & 0x01) {
            SPIFlash_WriteByte(writeAddress,buffer[0]);
            buffer++;
            writeAddress++;
            length--;
            if (length == 0)
                return;
            WriteFlashWriteEnable();
        }
        SPIFlash_SetAddr(SPIFLASH_WRITE_CMD, writeAddress);
        spi_xfer(SPIx, (uint8_t)~buffer[i++]);
        if (SPIFLASH_WRITE_SIZE == 2) {
            spi_xfer(SPIx, i < length ? ~buffer[i++] : 0xff);
        }
    } else {
        SPIFlash_SetAddr(0x02, writeAddress);
    }
    while(i < length) {
        if (SPIFLASH_USE_AAI) {
            CS_HI();
            WaitForWriteComplete();
            CS_LO();
            spi_xfer(SPIx, SPIFLASH_WRITE_CMD);
            if (SPIFLASH_WRITE_SIZE == 2) {
                //Writing 0xff will have no effect even if there is already data at this address
                spi_xfer(SPIx, i < length ? ~buffer[i++] : 0xff);
            }
        }
        spi_xfer(SPIx, (uint8_t)~buffer[i++]);
    }
    CS_HI();
    WaitForWriteComplete();
    WriteFlashWriteDisable();
}
/*
 *
 */
void SPIFlash_WriteByte(uint32_t writeAddress, const unsigned byte) {
    if (SPIFLASH_USE_AAI)
        DisableHWRYBY();
    WriteFlashWriteEnable();
    SPIFlash_SetAddr(0x02, writeAddress);
    spi_xfer(SPIx, (uint8_t)(~byte));
    CS_HI();
    WaitForWriteComplete();
    WriteFlashWriteDisable();
}
/*
 *
 */
void SPIFlash_ReadBytes(uint32_t readAddress, uint32_t length, uint8_t * buffer)
{
    uint32_t i;
    if (SPIFLASH_FAST_READ) {
        SPIFlash_SetAddr(0x0b, readAddress);
        spi_xfer(SPIx, 0);  // Dummy read
    } else {
        SPIFlash_SetAddr(0x03, readAddress);
    }

    for(i=0;i<length;i++)
    {
        buffer[i] = ~spi_xfer(SPIx, 0);
    }

    CS_HI();
}

int SPIFlash_ReadBytesStopCR(uint32_t readAddress, uint32_t length, uint8_t * buffer)
{
    uint32_t i;
    if (SPIFLASH_FAST_READ) {
        SPIFlash_SetAddr(0x0b, readAddress);
        spi_xfer(SPIx, 0);  // Dummy read
    } else {
        SPIFlash_SetAddr(0x03, readAddress);
    }

    for(i=0;i<length;i++)
    {
        buffer[i] = ~spi_xfer(SPIx, 0);
        if (buffer[i] == '\n') {
            i++;
            break;
        }
    }

    CS_HI();
    return i;
}
