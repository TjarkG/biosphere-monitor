cmake_minimum_required(VERSION 3.24)
project(biosphere-monitor C)

SET(MCU "atxmega32a4u")
SET(F_CPU "16000000")
SET(CMAKE_SYSTEM_NAME Generic)

SET(CMAKE_C_COMPILER /usr/bin/avr-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/avr-g++)
set (CMAKE_CXX_STANDARD 14)

SET(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

SET(CMAKE_CXX_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
SET(CMAKE_CXX_LINK_FLAGS "-mmcu=${MCU}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_executable(Microcontroller.elf main.c bme.c bme.h ATxmegaAux.h Spi_Flash.h)