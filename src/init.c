#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/vector.h>

// Based on libopencm3 project.
#include <stdint.h>

//extern unsigned _data_loadaddr, _data, _edata, _ebss, _stack;

typedef void (*vector_table_entry_t)(void);

typedef struct {
    unsigned int *initial_sp_value; /**< Initial stack pointer value. */
    vector_table_entry_t reset;
} small_vector_table_t;

void main(void);

void reset_handler(void)
{
	volatile unsigned *src, *dest;

	for (src = &_data_loadaddr, dest = &_data;
		dest < &_edata;
		src++, dest++) {
		*dest = *src;
	}

	while (dest < &_ebss) {
		*dest++ = 0;
	}

	/* Ensure 8-byte alignment of stack pointer on interrupts */
	/* Enabled by default on most Cortex-M parts, but not M3 r1 */
	SCB_CCR |= SCB_CCR_STKALIGN;

	/* Call the application's entry point. */
	(void)main();
}
// Vector table (bare minimal one)
__attribute__ ((section(".smallvectors")))
small_vector_table_t small_vector_table = {
	.initial_sp_value = &_stack,
	.reset = reset_handler,
};

