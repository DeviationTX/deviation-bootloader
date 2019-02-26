
/* Screen coordinates are as follows:
 * (128, 32)   ....   (0, 32)
 *   ...       ....     ...
 * (128, 63)   ....   (0, 63)
 * (128, 0)    ....   (0, 0)
 *   ...       ....     ...
 * (128, 31)   ....   (0, 31)
 */
static void draw_splash()
{
    unsigned ypos = ((64 - splash_height) / 2 + 7) / 8;
    unsigned xpos = (128 - splash_width) / 2;
    for (unsigned p = 0; p < LCD_PAGES; p++) {
        lcd_set_page_address(p);
        lcd_set_column_address(0);
        for(unsigned col = 0; col < PHY_LCD_WIDTH; col++) {
            if (col < xpos || p < ypos || col - xpos >= splash_width || p - ypos >= splash_height / 8) {
                LCD_Data(0x00);
            } else {
                LCD_Data(splash[(col - xpos) * (splash_height / 8) + (p - ypos)]);
            }
        }
    }
}
