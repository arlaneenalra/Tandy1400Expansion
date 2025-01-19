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
} frame_t;

#define BUS_FRAMES 10

isa_bus_state_t bus_frames[BUS_FRAMES];
uint8_t bus_frame = 0;

queue_t bus_queue;

void __not_in_flash_func(poll_bus)() {
  frame_t frame;
  isa_bus_state_t *isa;

  while(true) {
    frame.frame = bus_frame;
    isa = &(bus_frames[bus_frame]);

    isa_wait_for_addr();
    isa_read_operation(isa);
 
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
  frame_t frame;
  isa_bus_state_t *isa;

  set_sys_clock_khz(250000, true);

  stdio_init_all();
  init_isa_bus();

  // Setup the bus queue 
  queue_init(&bus_queue, sizeof(frame_t), BUS_FRAMES);

  // Start bus poller
  multicore_launch_core1(poll_bus);

  while (true) {
    if (queue_try_remove(&bus_queue, &frame)) {
      isa = &(bus_frames[frame.frame]);
      if (isa->addr & 0xFF000 == 0xB8000) {

        printf("OP: %i -> %i\n", frame.frame, frame.dropped);
        printf("0x%05X:0x%02X 0x%02X %c\n",
            isa->addr,
            isa->operation & BUS_OP_COMMAND,
            isa->data,
            isa->data);

      }
    }

    tight_loop_contents();
  }
}


