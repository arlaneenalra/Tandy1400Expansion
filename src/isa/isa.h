#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include <inttypes.h>

#define SET_BIT(b) (1u << b)

/**
 * Data GPIOs
 */
#define DATA_0 0
#define DATA_1 1
#define DATA_2 2
#define DATA_3 3
#define DATA_4 4
#define DATA_5 5
#define DATA_6 6
#define DATA_7 7

/**
 * Define bus control pins.
 */
#define OE_REQ 8

#define LIRQ 15
#define LDRQ 17
#define LIOREADY 16

#define OE_DATA 9
#define DIR_DATA 14
#define LATCH_DATA 18
#define OE_LATCH 19

#define BANK_0 10
#define BANK_1 11
#define BANK_2 12
#define BANK_3 13

#define GPIO20 20
#define GPIO21 21
#define GPIO22 22
#define GPIO26 26
#define GPIO27 27
#define GPIO28 28


/**
 * Special data pins
 */
#define AEN          DATA_4
#define ALE          DATA_5
#define RESET        DATA_0
#define TC           DATA_1
#define IO_READ      DATA_2
#define IO_WRITE     DATA_3
#define MEM_READ     DATA_4
#define MEM_WRITE    DATA_5
#define E_CLK        DATA_6
#define DACK         DATA_7

/**
 * IO Masks
 */

// Data operations only happen on pins 0-8
#define DATA_MASK          0x000000FF
#define CONTROL_MASK       0x000FFF00

// We control the first 20 GPIO pins
#define DATA_FUNCTION_MASK 0x000FFFFF

// Set things up so watching AEN and ALE
#define OE_MASK (  \
    SET_BIT(OE_DATA)     \
    | SET_BIT(OE_REQ)   \
    | SET_BIT(BANK_0)   \
    | SET_BIT(BANK_1)   \
    | SET_BIT(BANK_2)   \
    | SET_BIT(BANK_3)   \
    )

// Specific IO masks
#define BUS_AEN SET_BIT(AEN)
#define BUS_ALE SET_BIT(ALE)

#define BUS_RESET SET_BIT(RESET)
#define BUS_TC SET_BIT(TC)
#define BUS_E_IO_READ SET_BIT(IO_READ)
#define BUS_E_IO_WRITE SET_BIT(IO_WRITE)
#define BUS_E_MEM_READ SET_BIT(MEM_READ)
#define BUS_E_MEM_WRITE SET_BIT(MEM_WRITE)
#define BUS_E_CLK SET_BIT(E_CLK)
#define BUS_DACK SET_BIT(DACK)

#define BUS_OP_COMMAND (               \
    BUS_E_IO_READ | BUS_E_IO_WRITE |   \
    BUS_E_MEM_READ | BUS_E_MEM_WRITE   \
)

#define BUS_OP_READ (BUS_E_MEM_READ | BUS_E_IO_READ)
#define BUS_OP_WRITE (BUS_E_MEM_WRITE | BUS_E_IO_WRITE)


/**
 * Structure representing the current state of the attached ISA
 * bus. 
 *
 */
typedef struct isa_bus_state_type {
  uint32_t addr; // The bus actually uses 20bit addresses, but there
                 // are no 20bit types.
  
  uint8_t operation;
  uint8_t data;
} isa_bus_state_t;


/**
 * Initializes all of the GPIO pins etc for reading the isa bus.
 */
void init_isa_bus();

void isa_read_operation(isa_bus_state_t *bus);
void isa_wait_for_addr();

void __isa_set_bank(uint32_t bank);

#define isa_set_bank(b) __isa_set_bank(SET_BIT(b))


