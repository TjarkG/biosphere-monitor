cmake_minimum_required(VERSION 3.24)
project(biosphere-monitor)

set(CMAKE_CXX_STANDARD 14)

#CLI
add_executable(biosphere PC/cli.cpp PC/biosphere.cpp PC/biosphere.hpp PC/tty.cpp PC/tty.hpp PC/README.md.hpp)

#GUI
find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})

add_definitions(${GTKMM_CFLAGS})

add_executable(bioGui Gui/gui.cpp Gui/signals.hpp PC/biosphere.cpp PC/biosphere.hpp PC/tty.cpp PC/tty.hpp)

target_link_libraries(bioGui  ${GTKMM_LIBRARIES})

remove_definitions(${GTKMM_CFLAGS})

#AVR
set(MCU "atxmega32a4u")
set(F_CPU "16000000")
set(CMAKE_SYSTEM_NAME Generic)

set(DEFAULT_CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(CMAKE_C_COMPILER /usr/bin/avr-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/avr-g++)

set(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
set(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

set(DEFAULT_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
set(DEFAULT_CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS})
set(CMAKE_CXX_LINK_FLAGS "-mmcu=${MCU}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_executable(Microcontroller.elf
        Microcontroller/main.c Microcontroller/bme.c Microcontroller/bme.h
        Microcontroller/ATxmegaAux.h Microcontroller/spiFlash.h Microcontroller/spiFlash.c)

set(CMAKE_CXX_COMPILER ${DEFAULT_CMAKE_CXX_COMPILER})
set(CMAKE_CXX_FLAGS ${DEFAULT_CMAKE_CXX_FLAGS})
set(CMAKE_CXX_LINK_FLAGS ${DEFAULT_CMAKE_CXX_LINK_FLAGS})
