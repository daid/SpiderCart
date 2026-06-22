cmake_minimum_required(VERSION 3.13)
set(PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/board/)
set(PICO_BOARD spidercart)
include(pico_sdk_import.cmake)
project(SpiderCartFirmware)
pico_sdk_init()
add_compile_options(-Wall -Wextra -Wnull-dereference)
set(CMAKE_CXX_STANDARD 17)

find_package(Python3)
add_subdirectory(../loader loader)
add_custom_command(OUTPUT Loader.gb.inc COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/rom2inc.py ${CMAKE_CURRENT_BINARY_DIR}/loader/Loader.gb Loader.gb.inc DEPENDS Loader loader/Loader.gb ${CMAKE_CURRENT_SOURCE_DIR}/rom2inc.py)

add_executable(SpiderCart
    src/main.cpp
    src/usb_descriptors.c
    src/memchip.cpp
    src/mbc.cpp
    src/pins.cpp
    src/sddiskio.cpp
    src/romload.cpp
    inc/memchip.h
    inc/pins.h

    src/fatfs/diskio.h
    src/fatfs/ff.c
    src/fatfs/ff.h
    src/fatfs/ffconf.h
    src/fatfs/ffsystem.c
    src/fatfs/ffunicode.c
    
    src/lua/lapi.c
    src/lua/lauxlib.c
    src/lua/ldebug.c
    src/lua/lfunc.c
    src/lua/lvm.c
    src/lua/lgc.c
    src/lua/ldo.c
    src/lua/lobject.c
    src/lua/ltable.c
    src/lua/lstate.c
    src/lua/lstring.c
    src/lua/ltm.c
    src/lua/lopcodes.c
    src/lua/lmem.c
    src/lua/lparser.c
    src/lua/lzio.c
    src/lua/ldump.c
    src/lua/lundump.c
    src/lua/llex.c
    src/lua/lcode.c
    src/lua/lctype.c

    Loader.gb.inc
)
pico_generate_pio_header(SpiderCart ${CMAKE_CURRENT_LIST_DIR}/src/data_write.pio)
pico_generate_pio_header(SpiderCart ${CMAKE_CURRENT_LIST_DIR}/src/data_read.pio)

pico_enable_stdio_usb(SpiderCart 1)
pico_enable_stdio_uart(SpiderCart 0)

target_link_libraries(SpiderCart pico_stdlib hardware_i2c hardware_spi hardware_pio hardware_dma pico_multicore tinyusb_device tinyusb_board)
target_include_directories(SpiderCart PUBLIC src inc)
pico_add_extra_outputs(SpiderCart)

target_compile_definitions(SpiderCart PRIVATE
  PICO_STDIO_USB_DEFAULT_CRLF=0
  PICO_STDIO_DEFAULT_CRLF=0
  PICO_STDIO_ENABLE_CRLF_SUPPORT=0
  PICO_UART_ENABLE_CRLF_SUPPORT=0
  PICO_STDIO_USB_ENABLE_TINYUSB_INIT=1
  PICO_STDIO_USB_ENABLE_IRQ_BACKGROUND_TASK=1
  PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
  PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE=1200
  PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK=0
  PICO_CORE1_STACK_SIZE=0x200
)

add_executable(SpiderCartHWTest
    src/hwtest.cpp
    src/memchip.cpp
    src/pins.cpp
    inc/memchip.h
    inc/pins.h
)
pico_generate_pio_header(SpiderCartHWTest ${CMAKE_CURRENT_LIST_DIR}/src/data_write.pio)
pico_generate_pio_header(SpiderCartHWTest ${CMAKE_CURRENT_LIST_DIR}/src/data_read.pio)

pico_enable_stdio_usb(SpiderCartHWTest 1)
pico_enable_stdio_uart(SpiderCartHWTest 0)

target_link_libraries(SpiderCartHWTest pico_stdlib hardware_i2c hardware_spi hardware_pio hardware_dma pico_multicore)
target_include_directories(SpiderCartHWTest PRIVATE src inc)
pico_add_extra_outputs(SpiderCartHWTest)
