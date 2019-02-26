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

#define PHY_LCD_WIDTH 128
#define LCD_PAGES 8

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
    LCD_Cmd(0x81);
    int c = contrast * contrast * 255 / 100; //contrast should range from 0 to 255
    LCD_Cmd(c);
}

#include "draw_splash.h"

void LCD_Init()
{
    //Initialization is mostly done in SPI Flash
    //Setup CS as B.0 Data/Control = C.5
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    PORT_mode_setup(LCD_CSN_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    PORT_mode_setup(LCD_CMD_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    volatile int i = 0x8000;
    while(i) i--;
    lcd_display(0); //Display Off
    LCD_Cmd(0xD5);  //Set Display Clock Divide Ratio / OSC Frequency
    LCD_Cmd(0x80);  //Display Clock Divide Ratio / OSC Frequency
    LCD_Cmd(0xA8);  //Set Multiplex Ratio
    LCD_Cmd(0x3F);  //Multiplex Ratio for 128x64 (LCD_HEIGHT - 1)
    LCD_Cmd(0xD3);  //Set Display Offset
    LCD_Cmd(0x00);  //Display Offset (0)
    LCD_Cmd(0x40);  //Set Display Start Line (0)
    LCD_Cmd(0x8D);  //Set Charge Pump
    LCD_Cmd(0x10);  //Charge Pump (0x10 External, 0x14 Internal DC/DC)
    LCD_Cmd(0xA1);  //Set Segment Re-Map (Reversed)
    LCD_Cmd(0xC8);  //Set Com Output Scan Direction (Reversed)
    LCD_Cmd(0xDA);  //Set COM Hardware Configuration
    LCD_Cmd(0x12);  //COM Hardware Configuration
    LCD_Cmd(0xD9);  //Set Pre-Charge Period
    LCD_Cmd(0x4F);  //Set Pre-Charge Period (A[7:4]:Phase 2, A[3:0]:Phase 1)
    LCD_Cmd(0xDB);  //Set VCOMH Deselect Level
    LCD_Cmd(0x20);  //VCOMH Deselect Level (0x00 ~ 0.65 x VCC, 0x10 ~ 0.71 x VCC, 0x20 ~ 0.77 x VCC, 0x30 ~ 0.83 x VCC)
    LCD_Cmd(0xA4);  //Disable Entire Display On
    LCD_Cmd(0xA6);  //Set Normal Display (not inverted)
    LCD_Cmd(0x2E);  //Deactivate scroll
    lcd_display(1); //Display On
    draw_splash();
    LCD_Contrast(5);
}
