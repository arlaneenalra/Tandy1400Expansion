#include <strings.h>

#include "isa.h"

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
     gpio_set_input_hysteresis_enabled(i, false);
     gpio_set_slew_rate(i, GPIO_SLEW_RATE_FAST); 

     gpio_mask >>= 1;
  }

  // Set everything to inputs for now.
  gpio_set_dir_in_masked(DATA_FUNCTION_MASK);
  gpio_put_masked(DATA_FUNCTION_MASK, OE_MASK | SET_BIT(OE_LATCH));
  gpio_set_function_masked(DATA_FUNCTION_MASK, GPIO_FUNC_SIO);

  // Control lines are always outputs
  gpio_set_dir_out_masked(CONTROL_MASK);
}

/**
 * Bank output enables are all active low.
 * 
 */
void __isa_set_bank(uint32_t bank) {
  gpio_put_masked(OE_MASK, ~bank); 
}

/**
 * Reads an address from the bus.
 */
void isa_read_operation(isa_bus_state_t *bus) {
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

  isa_set_bank(BANK_3);
/*  // Wait for one of the command signals to go low.
  do {
    buf = gpio_get_all();
  } while(buf & BUS_E_CLK);*/
  
  bus->operation = buf;

  // If we're in a write operation, data should be valid at this point.
  //if ((buf & BUS_OP_WRITE) == BUS_OP_WRITE) {
  gpio_put(DIR_DATA, false);
  isa_set_bank(OE_DATA);
  bus->data = gpio_get_all() & DATA_MASK;
  //}
}


/**
 * Waits for ALE to go high then low.
 * Addresses are considered valid when ALE goes low.
 */
void isa_wait_for_addr() {
  isa_set_bank(BANK_2);
  do {} while (!gpio_get(AEN));
//  do {} while (gpio_get(AEN));
}
