# ESP32 Pomodoro Clock tasks

set shell := ["bash", "-cu"]

# List available recipes
default:
    @just --list

# Install pinned PlatformIO packages and libraries
install:
    pio pkg install

# Create the ignored local Wi-Fi configuration if it is missing
setup:
    @test -f include/wifi_config.h || cp include/wifi_config.example.h include/wifi_config.h
    @echo "Edit include/wifi_config.h with your Wi-Fi credentials."

# Run native Pomodoro engine tests
test:
    pio test -e native

# Build firmware for the JC4827W543
build:
    pio run -e jc4827w543

# Run all checks used by CI
check: test build

# Remove generated PlatformIO build files
clean:
    pio run -t clean

# Upload firmware to a connected board
upload:
    pio run -e jc4827w543 -t upload

# Open the serial monitor
monitor:
    pio device monitor -b 115200
