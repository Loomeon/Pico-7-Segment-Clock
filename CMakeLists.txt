cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(Pico_7_Segment_Clock)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(Pico_7_Segment_Clock
        main.c
        )

pico_enable_stdio_usb(Pico_7_Segment_Clock 1)
pico_enable_stdio_uart(Pico_7_Segment_Clock 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(Pico_7_Segment_Clock pico_stdlib hardware_rtc pico_multicore)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(Pico_7_Segment_Clock)

if (TARGET tinyusb_device)

 elseif(PICO_ON_DEVICE)
   message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in
   the SDK")
 endif()


