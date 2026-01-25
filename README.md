# OpenCineLightMeter

![OpenCineLightMeter](/assets/opencinelightmeter_2.jpg)

Have you noticed how ridiculously expensive incident light meters for photo and especially video are? $250 for a basic model is a lot! Especially, if you realize it's a light sensor, display and a microcontroller to do some math.

My YouTubeer "career" forces me to record videos quite often incident light meter was something I was missing. Especially when shooting b-roll on white or black backgrounds. One day I decided math can't be that complex, ordered VEML7700 digital light sensor, took ESP32 with OLED SSD1306 display and started hacking. Some time later OpenCineLightMeter came to be.

What it can do:
* measure incident light on a scene (flash light not supported)
* compute aperture based on ISO, shutter speed and used ND filter
* compute shutter speed based on ISO, aperture and used ND filter

![Light meter screen in aperture mode](/assets/opencinelightmeter_3.jpg)

And that's mostly all... Bear in mind, current version includes code and math for reflected light as well, but it's not working as expected yet, so this option is disabled from UI

## Required Parts

To build this light meter, you'll need the following components:

### Core Components
1. **ESP32 Development Board** - Main microcontroller (tested with esp32dev)
2. **VEML7700 Light Sensor** - Digital ambient light sensor (I2C interface)
3. **SSD1306 OLED Display** - 128x64 or 128x32 I2C OLED display
4. **5-Way Navigation Joystick** - For user interface (Mode, Up, Down, Left, Right)
5. **Tactile Push Button (Optional)** - HOLD button for additional functionality
6. **Breadboard or PCB** - For assembling the components
7. **Jumper Wires/Dupont Cables** - For connecting components

### Optional Components
- **Enclosure** - 3D printed or purchased case to house the meter
- **Battery/Power Supply** - USB power bank or LiPo battery with charging circuit

### Pin Connections
| Component | ESP32 GPIO |
|-----------|------------|
| OLED SDA | GPIO 4 |
| OLED SCL | GPIO 15 |
| OLED RESET | GPIO 16 |
| VEML7700 SDA | GPIO 4 (shared I2C) |
| VEML7700 SCL | GPIO 15 (shared I2C) |
| Joystick Mode | GPIO 26 |
| Joystick Up | GPIO 13 |
| Joystick Down | GPIO 12 |
| Joystick Left | GPIO 14 |
| Joystick Right | GPIO 27 |
| Button Hold (Optional) | GPIO 0 |

**Note:** VEML7700 and OLED share the same I2C bus (SDA/SCL pins).

Notes:
1. Tested only on ESP32, with some changes should work with ESP32-C3 and ESP32-S3
2. There is a plethora of SSD1306 equipped boards with ESP32, the one I use here, assumes:
    1. SDA -> GPIO4
    2. SCL -> GPIO16
    3. OLED RESET -> GPIO16 -> in needs to be pulled LOW and then HIGH during OLED operation
3. VEML7700 runs as a separate task with a 4Hz frequency