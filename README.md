# ESP32 Telemetry Display

An Assetto Corsa telemetry display built from two programs:

1. A Windows application reads Assetto Corsa's shared-memory physics data and sends it over UDP.
2. ESP32 firmware receives the packets over Wi-Fi and draws the gas and brake traces on a 128 × 160 ST7735 display.

The display firmware supports both full-framebuffer drawing and a hardware-scroll implementation. The current source uses the hardware-scroll path.

## Repository layout

| Path | Description |
| --- | --- |
| `app/` | Windows telemetry reader and UDP sender |
| `firmware/` | ESP-IDF firmware for the ESP32 |
| `common/protocol.h` | Packed telemetry packet shared by the sender and firmware |
| `test/display-timing/` | Captured display timing logs |
| `results.md` | Summary of the timing measurements |

## Hardware

The display driver is configured for an ST7735-compatible 128 × 160 SPI display. Connect the display to the ESP32 using these GPIO assignments:

| Display signal | ESP32 GPIO |
| --- | ---: |
| CS | 10 |
| MOSI | 11 |
| SCLK | 12 |
| DC | 9 |
| RESET | 8 |
| Backlight | 7 |

The firmware uses SPI mode 0 at 20 MHz. Check that these pins and the display voltage requirements are appropriate for your particular ESP32 board and display module before wiring.

## Build and flash the firmware

Install ESP-IDF and its toolchain, then open an ESP-IDF command prompt and run:

```powershell
idf.py -C firmware set-target esp32s3
idf.py -C firmware menuconfig
idf.py -C firmware build
idf.py -C firmware flash monitor
```

In `menuconfig`, set the Wi-Fi SSID and password under **Example Configuration**. The ESP32 joins that Wi-Fi network as a station; the Windows sender must be able to reach it on the same network. Keep personal Wi-Fi credentials out of committed configuration files.

The firmware listens for UDP telemetry on port **50000**.

## Build and run the Windows sender

Build with CMake and a C++17-capable Windows compiler:

```powershell
cmake -S app -B app/build
cmake --build app/build
```

Set `RECEIVER_IP` to the ESP32's IP address and `RECEIVER_PORT` to `50000`, then run the sender:

```powershell
$env:RECEIVER_IP = "192.168.1.123"
$env:RECEIVER_PORT = "50000"
.\app\build\telemetry_app.exe
```

Replace the example IP with the address assigned to your ESP32. The sender reads Assetto Corsa's `Local\acpmf_physics` shared-memory mapping and transmits the packed `PhysicsData` structure about every 20 ms. If the mapping is unavailable, the application offers keyboard-entered gas and brake values.

The packet structure is defined in `common/protocol.h` and contains a packet ID, gas, brake, fuel, gear, and RPM. Both programs must use the same protocol layout.

## Display timing

The timing summary in `results.md` reports:

| Rendering method | Samples | Draw average | Display push average |
| --- | ---: | ---: | ---: |
| Line-by-line framebuffer | 33 | 1,692 µs | 24,952 µs |
| Hardware scrolling | 32 | 33 µs | 551 µs |

For these runs, hardware scrolling substantially reduced both drawing and display-transfer time. The logs are in `test/display-timing/`.
