#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -S") 
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -S")

add_library(isa_lib
  isa.c
)

target_compile_options(isa_lib PRIVATE -save-temps -fverbose-asm)

target_link_libraries(isa_lib
  pico_stdlib
)
