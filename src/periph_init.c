#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/usb/usbd.h>
#include "hardware.h"

static inline void backlight_init()
{
    // rcc_periph_clock_enable(RCC_GPIOB);
    PORT_mode_setup(BACKLIGHT_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    PORT_pin_set(BACKLIGHT_PIN);
}

static inline void spiflash_init()
{
    /* Enable SPIx */
    // rcc_periph_clock_enable(RCC_GPIOB);  // Assume sck, mosi, miso all on same port
    PORT_mode_setup(FLASH_CSN_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    PORT_pin_set(FLASH_CSN_PIN);
}

static inline void lcd_init()
{
    //Initialization is mostly done in SPI Flash
    //Setup CS as B.0 Data/Control = C.5
    // rcc_periph_clock_enable(RCC_GPIOB);
    // rcc_periph_clock_enable(RCC_GPIOC);
    PORT_mode_setup(LCD_CSN_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    PORT_mode_setup(LCD_CMD_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
}

static inline void spi_init()
{
    /* Enable SPIx */
    // rcc_periph_clock_enable(RCC_SPIx);
    /* Enable GPIOA */
    // rcc_periph_clock_enable(RCC_GPIOA);
    /* Enable GPIOB */
    // rcc_periph_clock_enable(RCC_GPIOB);

    // PORT_mode_setup(FLASH_CSN_PIN,  GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    // PORT_mode_setup(LCD_CSN_PIN,  GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    // PORT_pin_set(FLASH_CSN_PIN);
    // PORT_pin_set(LCD_CSN_PIN);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  SPIx_SCK_PIN.pin | SPIx_MOSI_PIN.pin);
    PORT_mode_setup(SPIx_MISO_PIN, GPIO_MODE_INPUT,         GPIO_CNF_INPUT_FLOAT);

    /* Includes enable */
    spi_init_master(SPIx, 
                    SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, 
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPIx);
    spi_set_nss_high(SPIx);

    spi_enable(SPIx);
}

static inline void usb_init()
{
    // rcc_periph_clock_enable(RCC_GPIOB);
    // rcc_periph_clock_enable(RCC_GPIOA);
    // rcc_periph_clock_enable(RCC_OTGFS);

    // PORT_mode_setup(((struct mcu_pin){GPIOA, GPIO11 | GPIO12}), GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT);
    // PORT_mode_setup(((struct mcu_pin){GPIOB, GPIO10}), GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    // gpio_clear(GPIOB, GPIO10);
}

static inline void pwr_init()
{
    // rcc_periph_clock_enable(RCC_GPIOA);

    /* Pin controls power-down */
    PORT_mode_setup(PWR_ENABLE_PIN, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL);
    /* Enable GPIOA.2 to keep from shutting down */
    PORT_pin_set(PWR_ENABLE_PIN);

    /* When Pin goes high, the user turned off the Tx */
    PORT_mode_setup(PWR_SWITCH_PIN, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT);
}

void Periph_Init()
{
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_SPI1EN);
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_USBEN);
    // rcc_peripheral_enable_clock(&RCC_AHBENR, RCC_AHBENR_OTGFSEN);

    // GPIOA
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 
        PWR_ENABLE_PIN.pin
        );
    gpio_set(GPIOA, 
        PWR_ENABLE_PIN.pin
        );
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, 
        PWR_SWITCH_PIN.pin
      | GPIO11 | GPIO12    /* USB */
        );

    //GPIOB
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 
        BACKLIGHT_PIN.pin
      | LCD_CSN_PIN.pin
      | FLASH_CSN_PIN.pin
      | GPIO10 /* USB_ENABLE */
      );
    gpio_set(GPIOB, 
        BACKLIGHT_PIN.pin
      | LCD_CSN_PIN.pin
      | FLASH_CSN_PIN.pin
      );
    gpio_clear(GPIOB, 
      GPIO10 /* USB_ENABLE */
      );

    //GPIOC
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, 
        LCD_CMD_PIN.pin
      );
    gpio_set(GPIOC, 
        LCD_CMD_PIN.pin
      );
    // pwr_init();
    spi_init();
    //spiflash_init();
    //lcd_init();
    //backlight_init();
    //usb_init();
}
