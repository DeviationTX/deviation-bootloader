
#define PORT_pin_get(io)                  gpio_get(io.port, io.pin)
#define PORT_pin_set(io)                  gpio_set(io.port,io.pin)
#define PORT_pin_clear(io)                gpio_clear(io.port,io.pin)
#define PORT_mode_setup(io, mode, pullup) gpio_set_mode(io.port, mode, pullup, io.pin)

struct mcu_pin {
    uint32_t port;
    uint32_t pin;
};

#define SPIx        SPI1
#define RCC_SPIx    RCC_SPI1

#define PWR_SWITCH_PIN ((struct mcu_pin) {GPIOA, GPIO3})
#define PWR_ENABLE_PIN ((struct mcu_pin) {GPIOA, GPIO2})

#define BACKLIGHT_PIN ((struct mcu_pin) {GPIOB, GPIO1})

#define LCD_CMD_PIN   ((struct mcu_pin) {GPIOC, GPIO5})
#define LCD_CSN_PIN   ((struct mcu_pin) {GPIOB, GPIO0})
#define FLASH_CSN_PIN   ((struct mcu_pin) {GPIOB, GPIO2})
#define SPIx_SCK_PIN   ((struct mcu_pin) {GPIOA, GPIO5})
#define SPIx_MISO_PIN  ((struct mcu_pin) {GPIOA, GPIO6})
#define SPIx_MOSI_PIN  ((struct mcu_pin) {GPIOA, GPIO7})

#define RCC_MATRIX_COL  RCC_GPIOB
#define RCC_MATRIX_ROW  RCC_GPIOC
#define MATRIX_COL_PORT GPIOB
#define MATRIX_ROW_PORT GPIOC
#define MATRIX_COL_PIN  GPIO8
#if USE_EXT_BUTTON
    #define MATRIX_ROW_PIN  GPIO9     // EXT
#else
    #define MATRIX_ROW_PIN  GPIO7     // DN
#endif
#define MATRIX_COL_MASK (GPIO5 | GPIO6 | GPIO7 | GPIO8)
#define MATRIX_ROW_MASK (GPIO6 | GPIO7 | GPIO8 | GPIO9)

