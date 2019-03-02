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
#include <libopencm3/stm32/spi.h>
#include "hardware.h"
#include "splash.h"
#include <string.h>


#define LCD_WIDTH 128
#define LCD_HEIGHT 64

//The screen is 129 characters, but we'll only expoise 128 of them
#define PHY_LCD_WIDTH 129
#define LCD_PAGES 8

static void CS_HI()
{
    PORT_pin_set(LCD_CSN_PIN);
}

static void CS_LO()
{
    PORT_pin_clear(LCD_CSN_PIN);
}

static void CMD_MODE()
{
    PORT_pin_clear(LCD_CMD_PIN);
}

static void DATA_MODE()
{
    PORT_pin_set(LCD_CMD_PIN);
}

void LCD_Cmd(unsigned cmd) {
    CMD_MODE();
    CS_LO();
    spi_xfer(SPI1, cmd);
    CS_HI();
}

void LCD_Data(unsigned cmd) {
    DATA_MODE();
    CS_LO();
    spi_xfer(SPI1, cmd);
    CS_HI();
}

void lcd_display(uint8_t on)
{
    LCD_Cmd(0xAE | (on ? 1 : 0));
}

void lcd_set_page_address(uint8_t page)
{
    LCD_Cmd(0xB0 | (page & 0x07));
}

void lcd_set_column_address(uint8_t column)
{
    LCD_Cmd(0x10 | ((column >> 4) & 0x0F));  //MSB
    LCD_Cmd(column & 0x0F);                  //LSB
}

void lcd_set_start_line(int line)
{
  LCD_Cmd((line & 0x3F) | 0x40); 
}

void LCD_Contrast(unsigned contrast)
{
    //int data = 0x20 + contrast * 0xC / 10;
    LCD_Cmd(0x81);
    int c = contrast * 12 + 76; //contrast should range from ~72 to ~200
    LCD_Cmd(c);
}

#include "draw_splash.h"

void LCD_Init()
{
    LCD_Cmd(0xE2);  //Reset
    volatile int i = 0x8000;
    while(i) i--;
    lcd_display(0);     // Display Off
    LCD_Cmd(0xA6);      // Normal display
    LCD_Cmd(0xA4);      // All Points Normal
    LCD_Cmd(0xA0);      // Set SEG Direction (Normal)
    if (HAS_LCD_FLIPPED) {
        LCD_Cmd(0xC8);  // Set COM Direction (Reversed)
        LCD_Cmd(0xA2);  // Set The LCD Display Driver Voltage Bias Ratio (1/9)
    } else {
        LCD_Cmd(0xEA);  // ??
        LCD_Cmd(0xC4);  // Common Output Mode Scan Rate
    }
    LCD_Cmd(0x2C);      // Power Controller:Booster ON
    i = 0x8000;
    while(i) i--;
    LCD_Cmd(0x2E); //Power Controller: VReg ON
    i = 0x8000;
    while(i) i--;
    LCD_Cmd(0x2F); //Power Controller: VFollower ON
    i = 0x8000;
    while(i) i--;
    lcd_set_start_line(0);
    // Display data write (6)
    lcd_display(1);
    draw_splash();
    LCD_Contrast(5);
}
