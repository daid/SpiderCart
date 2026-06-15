cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/board/)
set(PICO_BOARD spidercart)
include(pico_sdk_import.cmake)
project(SpiderCartFirmware)
pico_sdk_init()

add_executable(SpiderCart
    src/main.cpp
    src/memchip.cpp
    src/mbc.cpp
    inc/memchip.h
    inc/pins.h
)
pico_generate_pio_header(SpiderCart ${CMAKE_CURRENT_LIST_DIR}/src/data_write.pio)
pico_generate_pio_header(SpiderCart ${CMAKE_CURRENT_LIST_DIR}/src/data_read.pio)

pico_enable_stdio_usb(SpiderCart 1)
pico_enable_stdio_uart(SpiderCart 0)

target_link_libraries(SpiderCart pico_stdlib hardware_i2c hardware_spi hardware_pio hardware_dma pico_multicore)
target_include_directories(SpiderCart PRIVATE src inc)
pico_add_extra_outputs(SpiderCart)

target_compile_definitions(SpiderCart PRIVATE
  PICO_STDIO_USB_DEFAULT_CRLF=0
  PICO_STDIO_DEFAULT_CRLF=0
  PICO_STDIO_ENABLE_CRLF_SUPPORT=0
  PICO_UART_ENABLE_CRLF_SUPPORT=0
)