# ESP32 OLED Pong Game

A Pong game for ESP32 with 128x64 OLED display (SSD1306). Built from scratch with a custom OLED driver.

## Hardware

- ESP32 (tested with ESP32-S3)
- OLED Display: 128x64 pixels, SSD1306 controller
- SPI interface
- Two buttons for control (GPIO 12 and 13)

## Wiring

| OLED | ESP32 |
|------|-------|
| MOSI | GPIO 23 |
| CLK  | GPIO 18 |
| CS   | GPIO 5 |
| DC   | GPIO 16 |
| RST  | GPIO 17 |

## Software

### Requirements

- ESP-IDF (Version 4.0+)
- CMake 3.16+

### Build

```bash
cd Oled
idf.py set-target esp32
idf.py build
```

### Flash

```bash
idf.py -p COMX flash monitor
```

Replace `COMX` with your serial port.

## Controls

- **Left button (GPIO 13)**: Move paddle left
- **Right button (GPIO 12)**: Move paddle right

## Project Structure

```
Oled/
├── main/
│   ├── main.c      # Main program (Pong game)
│   ├── oled.c     # OLED driver
│   └── oled.h     # OLED header
├── CMakeLists.txt
└── sdkconfig      # ESP-IDF configuration
```

## License

MIT License