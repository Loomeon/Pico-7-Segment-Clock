cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(Pico_7_Segment_Clock)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add the executable to the Project
add_executable(Pico_7_Segment_Clock
        main.c
        )

# Add all the needed libraries
target_link_libraries(Pico_7_Segment_Clock pico_stdlib hardware_rtc pico_multicore)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(Pico_7_Segment_Clock)



