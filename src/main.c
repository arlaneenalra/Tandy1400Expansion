#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "isa/isa.h"

typedef struct frame_type {
  uint8_t frame; 
  uint32_t dropped;
  isa_bus_state_t isa; 
} frame_t;

#define BUS_FRAMES 100

uint8_t bus_frame = 0;

queue_t bus_queue;

void __not_in_flash_func(poll_bus)() {
  frame_t frame;

  while(true) {
    frame.frame = bus_frame;

//    isa_wait_for_addr(isa);
//    isa_read_operation(isa);
      isa_read_bus(&frame.isa);
 
    // Did we drop a frame?
    if (queue_try_add(&bus_queue, &frame)) {
      bus_frame = (bus_frame + 1) % BUS_FRAMES;
      frame.dropped = 0;
    } else {
      frame.dropped++;
    }
  }
}

int main() {
  uint8_t count = 0;
  frame_t frame;
  isa_bus_state_t *isa;

  set_sys_clock_khz(250000, true);

  // Setup LED
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  stdio_init_all();
  init_isa_bus();

  // Setup the bus queue 
  queue_init(&bus_queue, sizeof(frame_t), BUS_FRAMES);

  // Start bus poller
  multicore_launch_core1(poll_bus);

  gpio_put(PICO_DEFAULT_LED_PIN, true);

  while (true) {
    if (queue_try_remove(&bus_queue, &frame)) {
      isa = &frame.isa;
      if ((isa->addr < 0xBC000u) && (isa->addr > 0xB8000u)) {
//      if (isa->addr > 0xFFF00u) {      
        printf("OP: %i -> %i\n",
          frame.frame, frame.dropped);
        printf("0x%05X:0b%08b 0x%02X\n",
          isa->addr,
          isa->operation & BUS_OP_COMMAND,
          isa->data);
      }
    }
    tight_loop_contents();
  }
}


