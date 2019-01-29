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
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/fsmc.h>
#include <libopencm3/stm32/spi.h>
#include "common.h"
#include "lcd.h"

#define LCD_REG_ADDR  ((uint32_t)0x6C000000)    /* Register Address */
#define LCD_DATA_ADDR  ((uint32_t)(LCD_REG_ADDR + 2)) /* Data Address */

#define LCD_REG  *(volatile uint16_t *)(LCD_REG_ADDR)
#define LCD_DATA *(volatile uint16_t *)(LCD_DATA_ADDR)

#define CS_HI() gpio_set(GPIOB, GPIO1)
#define CS_LO() gpio_clear(GPIOB, GPIO1)


#define LCD_00_PRODCODE   0x00 // Product Code Register
#define LCD_02_CONF_READ  0x02 // Configuration Readback Register
#define LCD_04_PLL_DDIV   0x04 // PLL D-Divider Register
#define LCD_06_PLL0       0x06 // PLL Setting Register 0
#define LCD_08_PLL1       0x08 // PLL Setting Register 1
#define LCD_0A_PLL2       0x0A // PLL Setting Register 2
#define LCD_0C_NDIV       0x0C // PLL N-Divider Register
#define LCD_0E_SSCON1     0x0E // SS Control Register 0
#define LCD_10_SSCON1     0x10 // SS Control Register 1
#define LCD_12_CLK_SRC    0x12 // Clock Source Select Register
#define LCD_14_PANELTYPE  0x14 // LCD Panel Type Register
#define LCD_16_HORIZ_WID  0x16 // Horizontal Display Width Register (HDISP)
#define LCD_18_HORIZ_NDP  0x18 // Horizontal Non-Display Period Register (HNDP)
#define LCD_1A_VERT_HGHT0  0x1A // Vertical Display Height Register 0 (VDISP)
#define LCD_1C_VERT_HGHT1  0x1C // Vertical Display Height Register 1 (VDISP)
#define LCD_1E_VERT_NDP   0x1E // Vertical Non-Display Period Register (VNDP)
#define LCD_20_PHS_HSW    0x20 // PHS Pulse Width Register (HSW)
#define LCD_22_PHS_HPS    0x22 // PHS Pulse Start Position Register (HPS)
#define LCD_24_PVS_VSW    0x24 // PVS Pulse Width Register (VSW)
#define LCD_26_PVS_VPS    0x26 // PVS Pulse Start Position Register (VPS)
#define LCD_28_PCLK_POL   0x28 // PCLK Polarity Register
#define LCD_2A_DISPMODE   0x2A // Display Mode Register
#define LCD_2C_PIP1_DISP_START0  0x2C // PIP1 Display Start Address Register 0
#define LCD_2E_PIP1_DISP_START1  0x2E // PIP1 Display Start Address Register 1
#define LCD_30_PIP1_DISP_START2  0x30 // PIP1 Display Start Address Register 2
#define LCD_32_PIP1_XSTART   0x32 // PIP1 Window X Start Position Register
#define LCD_34_PIP1_YSTART0  0x34 // PIP1 Window Y Start Position Register 0
#define LCD_36_PIP1_YSTART1  0x36 // PIP1 Window Y Start Position Register 1
#define LCD_38_PIP1_XEND     0x38 // PIP1 Window X End Position Register
#define LCD_3A_PIP1_YEND0    0x3A // PIP1 Window Y End Position Register 0
#define LCD_3C_PIP1_YEND1    0x3C // PIP1 Window Y End Position Register 1
#define LCD_3E_PIP2_DISP_START0  0x3E // PIP2 Display Start Address Register 0
#define LCD_40_PIP2_DISP_START1  0x40 // PIP2 Display Start Address Register 1
#define LCD_42_PIP2_DISP_START2  0x42 // PIP2 Display Start Address Register 2
#define LCD_44_PIP2_XSTART   0x44 // PIP2 Window X Start Position Register
#define LCD_46_PIP2_YSTART0  0x46 // PIP2 Window Y Start Position Register 0
#define LCD_48_PIP2_YSTART1  0x48 // PIP2 Window Y Start Position Register 1
#define LCD_4A_PIP2_XEND     0x4A // PIP2 Window X End Position Register
#define LCD_4C_PIP2_YEND0    0x4C // PIP2 Window Y End Position Register 0
#define LCD_4E_PIP2_YEND1    0x4E // PIP2 Window Y End Position Register 1
#define LCD_50_DISPCON       0x50 // Display Control Register
#define LCD_52_INP_MODE      0x52 // Input Mode Register
#define LCD_54_TRANS_KEY_RED    0x54 // Transparency Key Color Red Register
#define LCD_56_TRANS_KEY_GREEN  0x56 // Transparency Key Color Green Register
#define LCD_58_TRANS_KEY_BLUE   0x58 // Transparency Key Color Blue Register
#define LCD_5A_WRWIN_XSTART   0x5A // Write Window X Start Position Register
#define LCD_5C_WRWIN_YSTART0  0x5C // Write Window Y Start Position Register 0
#define LCD_5E_WRWIN_YSTART1  0x5E // Write Window Y Start Position Register 1
#define LCD_60_WRWIN_XEND     0x60 // Write Window X End Position Register
#define LCD_62_WRWIN_YEND0    0x62 // Write Window Y End Position Register 0
#define LCD_64_WRWIN_YEND1    0x64 // Write Window Y End Position Register 1
#define LCD_66_MEM_PORT0      0x66 // Memory Data Port Register 0
#define LCD_67_MEM_PORT1      0x67 // Memory Data Port Register 1
#define LCD_68_PWR_SAVE       0x68 // Power Save Register
#define LCD_6A_NONDISP_PERIOD  0x6A // Non-Display Period Control / Status Register
#define LCD_6C_GPOUT0         0x6C // General Purpose Output Register 0
#define LCD_6E_GPOUT1         0x6E // General Purpose Output Register 1
#define LCD_70_PWM_CTRL       0x70 // PWM Control Register
#define LCD_72_PWM_HIGH0      0x72 // PWM High Duty Cycle Register 0
#define LCD_74_PWM_HIGH1      0x74 // PWM High Duty Cycle Register 1
#define LCD_76_PWM_HIGH2      0x76 // PWM High Duty Cycle Register 2
#define LCD_78_PWM_HIGH3      0x78 // PWM High Duty Cycle Register 3
#define LCD_7A_PWM_LOW0       0x7A // PWM Low Duty Cycle Register 0
#define LCD_7C_PWM_LOW1       0x7C // PWM Low Duty Cycle Register 1
#define LCD_7E_PWM_LOW2       0x7E // PWM Low Duty Cycle Register 2
#define LCD_80_PWM_LOW3       0x80 // PWM Low Duty Cycle Register 3
#define LCD_82_SDRAM_CTRL     0x82 // SDRAM Control Register
#define LCD_84_SDRAM_STATUS0  0x84 // SDRAM Status Register 0
#define LCD_86_SDRAM_STATUS1  0x86 // SDRAM Status Register 1
#define LCD_88_SDRAM_MRS0     0x88 // SDRAM MRS Value Register 0
#define LCD_8A_SDRAM_MRS1     0x8A // SDRAM MRS Value Register 1
#define LCD_8C_SDRAM_RFRSH_CNT0  0x8C // SDRAM Refresh Counter Register 0
#define LCD_8E_SDRAM_RFRSH_CMT1  0x8E // SDRAM Refresh Counter Register 1
#define LCD_90_SDRAM_BUFSIZE  0x90 // SDRAM Write Buffer Memory Size Register 0
#define LCD_92_SDRAM_DEBUG    0x92 // SDRAM Debug Register
#define LCD_94_ALPHA_CTRL     0x94 // Alpha-Blend Control Register
#define LCD_96_RESERVED       0x96 // is Reserved
#define LCD_98_ALPHA_HSIZE    0x98 // Alpha-Blend Horizontal Size Register
#define LCD_9A_ALPHA_VSIZE0   0x9A // Alpha-Blend Vertical Size Register 0
#define LCD_9C_ALPHA_VSIZE1   0x9C // Alpha-Blend Vertical Size Register 1
#define LCD_9E_ALPHA_VALUE    0x9E // Alpha-Blend Value Register
#define LCD_A0_ALPHA_INP1_START0  0xA0 // Alpha-Blend Input Image 1 Start Address Register 0
#define LCD_A2_ALPHA_INP1_START1  0xA2 // Alpha-Blend Input Image 1 Start Address Register 1
#define LCD_A4_ALPHA_INP1_START2  0xA4 // Alpha-Blend Input Image 1 Start Address Register 2
#define LCD_A6_ALPHA_INP2_START0  0xA6 // Alpha-Blend Input Image 2 Start Address Register 0
#define LCD_A8_ALPHA_INP2_START1  0xA8 // Alpha-Blend Input Image 2 Start Address Register 1
#define LCD_AA_ALPHA_INP2_START2  0xAA // Alpha-Blend Input Image 2 Start Address Register 2
#define LCD_AC_ALPHA_OUT_START0   0xAC // Alpha-Blend Output Image Start Address Register 0
#define LCD_AE_ALPHA_OUT_START1   0xAE // Alpha-Blend Output Image Start Address Register 1
#define LCD_B0_ALPHA_OUT_START2   0xB0 //Alpha-Blend Output Image Start Address Register 2
#define LCD_B2_INTRPT_CTRL    0xB2 // Interrupt Control Register
#define LCD_B4_INTRPT_STATUE  0xB4 // Interrupt Status Register
#define LCD_B6_INTRPT_CLEAR   0xB6 // Interrupt Clear Register


//See this thread: http://www.deviationtx.com/forum/6-general-discussions/4278-please-help-devo-12s-fonts-is-out-of-wack
//The new Devo12 uses a different rev of the Epson chip, which doesn't like our transparency value
//Note that the new value could be accidently triggered by a bad color choice since it is in the 15bit colorspace
//#define TRANSPARENT 0xFEFEFE
#define TRANSPARENT 0xA850A8 //FIXME
static u8 invert;
static u16 xpos, ypos, xstart, xend;

extern void PARFlash_Init();

void WRITE_PX(unsigned int c) {
    LCD_DATA = (c >> 16) & 0xff;
    LCD_DATA = c & 0xFFFF;
}

void lcd_cmd(uint8_t addr, uint8_t data)
{
    LCD_REG = addr;
    LCD_DATA = data;
}

u8 lcd_read_reg(uint8_t addr)
{
    LCD_REG = addr;
    return LCD_DATA;
}

u32 CONVERT_COLOR(u32 color) {
    if (color == TRANSPARENT_COLOR)
        color = TRANSPARENT;
    else 
        color = ((color >> 11) << 19)        //RED
            | (((color >> 5) & 0x3F) << 10)  //GREEN
            | ((color & 0x1F) << 3);         //BLUE
    return color;

}
void LCD_DrawPixel(unsigned int color)
{
    if(xpos == xstart) {
        for(int i = xpos; i & 0x07; i--) {
            WRITE_PX(TRANSPARENT);
        }
    }
    color = CONVERT_COLOR(color);
    WRITE_PX(color);
    xpos++;
    if (xpos > xend) {
        for(int i = xpos; i & 0x07; i++) {
            WRITE_PX(TRANSPARENT);
        }
        xpos = xstart;
    }
}

void LCD_DrawPixelXY(unsigned int x, unsigned int y, unsigned int color)
{
    LCD_REG = LCD_5A_WRWIN_XSTART;
    LCD_DATA = (x / 4) & 0xFE;
    LCD_DATA = y >> 2;
    LCD_DATA = y & 0x03;
    LCD_REG = LCD_66_MEM_PORT0;
    color = CONVERT_COLOR(color);
    u32 pixels[8] = {TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT,
                     TRANSPARENT, TRANSPARENT, TRANSPARENT, TRANSPARENT};
    pixels[x % 8] = color;
    for(int i = 0; i < 8; i++)
        WRITE_PX(pixels[i]);
}

void LCD_DrawStart(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, enum DrawDir dir)
{
  if (dir == DRAW_SWNE) {
    invert = 1;
    lcd_cmd(LCD_52_INP_MODE, 0x0B);
    ypos = y1;
  } else {
    invert = 0;
    ypos = y0;
    lcd_cmd(LCD_52_INP_MODE, 0x08);
  }
  xpos = x0;
  LCD_REG = LCD_5A_WRWIN_XSTART;
  LCD_DATA = (x0 / 4) & 0xFE;
  LCD_DATA = y0 >> 2;
  LCD_DATA = y0 & 0x03;
  LCD_DATA = (x1 / 4) & 0xFE;
  LCD_DATA = y1 >> 2;
  LCD_DATA = y1 & 0x03; 
  //printf("LCD_DrawStart: (%d, %d) - (%d, %d)\n", x0, y0, x1, y1);
  xstart = x0;
  xend = x1;
  return;
}

void LCD_DrawStop(void)
{
    lcd_cmd(LCD_52_INP_MODE, 0x08);
    return;
}

void lcd_clear(unsigned int color)
{
    LCD_REG = LCD_5A_WRWIN_XSTART;
    LCD_DATA = 0;
    LCD_DATA = 0;
    LCD_DATA = 0;
    LCD_DATA = 0x76;
    LCD_DATA = 0x43;
    LCD_DATA = 0x03;
    color = CONVERT_COLOR(color);
    for(int i= 0; i < 480 * 272; i++) {
        WRITE_PX(color);
    }
}

void SPILCD_SetRegister(u8 address, u16 data)
{
    CS_LO();
    spi_xfer(SPI1, 0x70); //Address
    spi_xfer(SPI1, 0x00);
    spi_xfer(SPI1, address);
    CS_HI();
    usleep(1);
    CS_LO();
    spi_xfer(SPI1, 0x72); //Write Data
    spi_xfer(SPI1, data >> 8);
    spi_xfer(SPI1, data & 0xFF);
    CS_HI();
}

u16 SPILCD_ReadRegister(u8 address)
{
    CS_LO();
    spi_xfer(SPI1, 0x70); //Address
    spi_xfer(SPI1, 0x00);
    spi_xfer(SPI1, address);
    CS_HI();
    usleep(1);
    CS_LO();
    spi_xfer(SPI1, 0x73); //Write Data
    u16 out = spi_xfer(SPI1, 0);
    out <<= 8;
    out |= spi_xfer(SPI1, 0);
    CS_HI();
    return out;
}

void SPILCD_Init()
{
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
    CS_HI();
    SPILCD_SetRegister(0x01, 0x7946);
    SPILCD_SetRegister(0x02, 0x2037);
    SPILCD_SetRegister(0x03, 0xDB30);
    SPILCD_SetRegister(0x04, 0x00AF);
    SPILCD_SetRegister(0x05, 0x1FCC);
    SPILCD_SetRegister(0x06, 0x372E);
    SPILCD_SetRegister(0x07, 0x000C);
    SPILCD_SetRegister(0x08, 0x002B);
    SPILCD_SetRegister(0x09, 0x4008);
    SPILCD_SetRegister(0x0F, 0x0140);
    SPILCD_SetRegister(0x10, 0x0301);
    SPILCD_SetRegister(0x11, 0x0201);
    SPILCD_SetRegister(0x12, 0x0005);
    SPILCD_SetRegister(0x13, 0x0101);
    SPILCD_SetRegister(0x14, 0x0207);
    SPILCD_SetRegister(0x15, 0x0300);
    SPILCD_SetRegister(0x16, 0x0201);
    SPILCD_SetRegister(0x17, 0x0306);
    SPILCD_SetRegister(0x18, 0x0C02);
    SPILCD_SetRegister(0x19, 0x0905);
}

void LCD_Init()
{
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPEEN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPFEN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPGEN);
    rcc_peripheral_enable_clock(&RCC_AHBENR, RCC_AHBENR_FSMCEN);

    // FSMC Data/Address pins
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO14 | GPIO15);

    gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO7 | GPIO8 | GPIO9 | GPIO10 | GPIO11 | GPIO12 | GPIO13 | GPIO14 | GPIO15);

    gpio_set_mode(GPIOF, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5 | GPIO12 | GPIO13 | GPIO14 | GPIO15);

    gpio_set_mode(GPIOG, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO0 | GPIO1 | GPIO2 | GPIO3 | GPIO4 | GPIO5);


    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO11 | GPIO12 | GPIO13);

    gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO3 | GPIO4 | GPIO5 | GPIO6);

    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO4 | GPIO5);

    gpio_set_mode(GPIOG, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO9 | GPIO12);

    gpio_set_mode(GPIOD, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_PULL_UPDOWN,
                  GPIO6);
    gpio_set(GPIOD, GPIO6); //Pull Up

    /* Extended mode, write enable, 16 bit access, bank enabled */
    FSMC_BCR4 = FSMC_BCR_MWID | FSMC_BCR_WREN | FSMC_BCR_MBKEN;

    /* Read & write timings */
    FSMC_BTR4  = FSMC_BTR_DATASTx(2) | FSMC_BTR_ADDHLDx(0) | FSMC_BTR_ADDSETx(1) | FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B);
    FSMC_BWTR4 = FSMC_BTR_DATASTx(2) | FSMC_BTR_ADDHLDx(0) | FSMC_BTR_ADDSETx(1) | FSMC_BTR_ACCMODx(FSMC_BTx_ACCMOD_B);
    PARFlash_Init();

    //SPILCD_Init();

    lcd_cmd(LCD_2A_DISPMODE, 0x00);
    lcd_cmd(LCD_68_PWR_SAVE, 0x00);
    lcd_cmd(LCD_04_PLL_DDIV, 0x19);
    lcd_cmd(LCD_06_PLL0, 0x01);
    lcd_cmd(LCD_08_PLL1, 0x00);
    lcd_cmd(LCD_0C_NDIV, 0x14);
    lcd_cmd(LCD_12_CLK_SRC, 0x00);
    lcd_cmd(LCD_04_PLL_DDIV, 0x9A);
    lcd_cmd(LCD_0E_SSCON1, 0x3F);
    usleep(1000);
    lcd_cmd(LCD_12_CLK_SRC, 0x80);
    lcd_cmd(LCD_14_PANELTYPE, 0x02);
    lcd_cmd(LCD_16_HORIZ_WID, 0x3B);
    lcd_cmd(LCD_18_HORIZ_NDP, 0x16);
    lcd_cmd(LCD_1A_VERT_HGHT0, 0x0F);
    lcd_cmd(LCD_1C_VERT_HGHT1, 0x01);
    lcd_cmd(LCD_1E_VERT_NDP, 0x06);
    lcd_cmd(LCD_20_PHS_HSW, 0x27);
    lcd_cmd(LCD_22_PHS_HPS, 0x04);
    lcd_cmd(LCD_24_PVS_VSW, 0x09);
    lcd_cmd(LCD_26_PVS_VPS, 0x04);
    lcd_cmd(LCD_28_PCLK_POL, 0x00);
    lcd_cmd(LCD_82_SDRAM_CTRL, 0x00);
    lcd_cmd(LCD_8C_SDRAM_RFRSH_CNT0, 0xFF);
    lcd_cmd(LCD_8E_SDRAM_RFRSH_CMT1, 0x03);
    lcd_cmd(LCD_90_SDRAM_BUFSIZE, 0x20);
    lcd_cmd(LCD_68_PWR_SAVE, 0xE8);
    lcd_cmd(LCD_68_PWR_SAVE, 0x00);
    lcd_cmd(LCD_68_PWR_SAVE, 0x01);
    lcd_cmd(LCD_84_SDRAM_STATUS0, 0x82);
    while(! (lcd_read_reg(LCD_86_SDRAM_STATUS1) & 0x02));
    lcd_cmd(LCD_52_INP_MODE, 0x00);
    lcd_cmd(LCD_2C_PIP1_DISP_START0, 0x00);
    lcd_cmd(LCD_2E_PIP1_DISP_START1, 0x00);
    lcd_cmd(LCD_30_PIP1_DISP_START2, 0x10);
    lcd_cmd(LCD_32_PIP1_XSTART, 0x00);
    lcd_cmd(LCD_34_PIP1_YSTART0, 0x00);
    lcd_cmd(LCD_36_PIP1_YSTART1, 0x00);
    lcd_cmd(LCD_38_PIP1_XEND, 0x76);
    lcd_cmd(LCD_3A_PIP1_YEND0, 0x43);
    lcd_cmd(LCD_3C_PIP1_YEND1, 0x03);
    lcd_cmd(LCD_3E_PIP2_DISP_START0, 0x00);
    lcd_cmd(LCD_40_PIP2_DISP_START1, 0x00);
    lcd_cmd(LCD_42_PIP2_DISP_START2, 0x18);
    lcd_cmd(LCD_44_PIP2_XSTART, 0x00);
    lcd_cmd(LCD_46_PIP2_YSTART0, 0x00);
    lcd_cmd(LCD_48_PIP2_YSTART1, 0x00);
    lcd_cmd(LCD_4A_PIP2_XEND, 0x76);
    lcd_cmd(LCD_4C_PIP2_YEND0, 0x43);
    lcd_cmd(LCD_4E_PIP2_YEND1, 0x03);
    lcd_cmd(LCD_52_INP_MODE, 0x08);
    lcd_cmd(LCD_54_TRANS_KEY_RED,   (TRANSPARENT >> 16) & 0xff);
    lcd_cmd(LCD_56_TRANS_KEY_GREEN, (TRANSPARENT >>  8) & 0xff);
    lcd_cmd(LCD_58_TRANS_KEY_BLUE,  (TRANSPARENT >>  0) & 0xff);
    lcd_cmd(LCD_5A_WRWIN_XSTART, 0x00);
    lcd_cmd(LCD_5C_WRWIN_YSTART0, 0x00);
    lcd_cmd(LCD_5E_WRWIN_YSTART1, 0x00);
    lcd_cmd(LCD_60_WRWIN_XEND, 0x76);
    lcd_cmd(LCD_62_WRWIN_YEND0, 0x43);
    lcd_cmd(LCD_64_WRWIN_YEND1, 0x03);
    lcd_cmd(LCD_6A_NONDISP_PERIOD, 0x03);
    lcd_cmd(LCD_2A_DISPMODE, 0x01);
    lcd_cmd(LCD_50_DISPCON, 0x80);
    lcd_clear(0x001F);
    lcd_cmd(LCD_6E_GPOUT1, 0x08);
}

void LCD_Sleep()
{
}
