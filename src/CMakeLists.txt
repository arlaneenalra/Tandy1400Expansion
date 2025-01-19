
add_subdirectory(isa)

add_executable(sniffer
  main.c
)

pico_enable_stdio_usb(sniffer 1)
pico_enable_stdio_uart(sniffer 0)

pico_add_extra_outputs(sniffer)

target_include_directories(sniffer PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/isa)

target_link_libraries(sniffer
  pico_stdlib
  pico_multicore
  isa_lib
)
