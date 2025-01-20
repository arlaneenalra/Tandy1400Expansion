#include <strings.h>

#include "isa.h"

#define ISA_PIO pio0
#define ISA_ADDR_SM 0

/**
 * ISA reference timings https://gist.github.com/PhirePhly/2209518
 */

void init_isa_bus() {
  // Disable pulls
  uint32_t gpio_mask = DATA_FUNCTION_MASK;
  for (int i = 0; i < 32; i ++ ) {
    // We're assuming a continuous block of gpios 
    if (!(gpio_mask & 1u )) {
      break;
    }
      
     gpio_set_pulls(i, false, false);
     //gpio_set_input_hysteresis_enabled(i, false);
     gpio_set_slew_rate(i, GPIO_SLEW_RATE_FAST); 

     gpio_mask >>= 1;
  }

  // Set everything to inputs for now.
  gpio_set_dir_in_masked(DATA_FUNCTION_MASK);
  gpio_put_masked(CONTROL_MASK, OE_MASK | SET_BIT(OE_LATCH));
  gpio_set_function_masked(CONTROL_MASK, GPIO_FUNC_SIO);

  // Control lines are always outputs
  gpio_set_dir_out_masked(CONTROL_MASK);

  gpio_put(DIR_DATA, false);

  // Setup the isa address reader pio.
  uint isa_addr_offset = pio_add_program(ISA_PIO, &isa_addr_program);
  pio_sm_claim(ISA_PIO, ISA_ADDR_SM);
  isa_addr_pio_init(ISA_PIO, ISA_ADDR_SM, isa_addr_offset);
}

/**
 * Bank output enables are all active low.
 * 
 */
void __not_in_flash_func(__isa_set_bank)(uint32_t bank) {
  gpio_put_masked(OE_MASK, ~bank);

  pico_default_asm_volatile("nop");
  pico_default_asm_volatile("nop");
  pico_default_asm_volatile("nop");
  pico_default_asm_volatile("nop");
}

/**
 * Reads an address from the bus.
 */
void __not_in_flash_func(isa_read_operation)(isa_bus_state_t *bus) {
  uint32_t buf, buf1, buf2;

  // Pull the 20bit address
  isa_set_bank(BANK_0);
  buf = gpio_get_all();

  isa_set_bank(BANK_1);
  buf1 = gpio_get_all();

  isa_set_bank(BANK_2);
  buf2 = gpio_get_all();

  bus->addr = (buf & DATA_MASK) |
    ((buf1 & DATA_MASK) << 8) |
    ((buf1 & 0x0F) << 16);
//  bus->addr = buf & DATA_MASK;

  // If we're in a write operation, data should be valid at this point.
  gpio_put(DIR_DATA, false);
  isa_set_bank(OE_DATA);
  bus->data = gpio_get_all() & DATA_MASK;
}


/**
 * Waits for ALE to go high then low.
 * Addresses are considered valid when ALE goes low.
 */
void __not_in_flash_func(isa_wait_for_addr)(isa_bus_state_t *bus) {
  uint32_t bank2, bank1, bank0;

  isa_set_bank(BANK_2);
  // Wait for rising edge
  do {
    tight_loop_contents();
    bank2 = gpio_get_all();
  } while ((bank2 & BUS_ALE) != BUS_ALE);

  // Wait for falling edge
  do {
    tight_loop_contents();
    bank2 = gpio_get_all();
  } while ((bank2 & BUS_ALE) != 0);

  // Pull the 20bit address
  isa_set_bank(BANK_0);
  bank0 = gpio_get_all();

  isa_set_bank(BANK_1);
  bank1 = gpio_get_all();

  bus->addr = (bank0 & DATA_MASK) |
    ((bank1 & DATA_MASK) << 8) |
    ((bank2 & 0x0F) << 16);

  isa_set_bank(BANK_3);
  bus->operation = gpio_get_all() & DATA_MASK;

  gpio_put(DIR_DATA, false);
  isa_set_bank(OE_DATA);
  bus->data = gpio_get_all() & DATA_MASK;
}

/**
 * Use PIO to read an address and data byte
 */
void __not_in_flash_func(isa_read_bus)(isa_bus_state_t * bus) {
  uint32_t raw_bus = pio_sm_get_blocking(ISA_PIO, ISA_ADDR_SM);

  bus->addr = (raw_bus &  0x0FFFFF00) >> 8; // mask off address
  bus->operation = (raw_bus & 0x000000FF); // Mask of operation
  bus->data = 0x00; // not reading data right now. 
}
