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

Notes:
1. Tested only on ESP32, with some changes should work with ESP32-C3 and ESP32-S3
2. There is a plethora of SSD1306 equipped boards with ESP32, the one I use here, assumes:
    1. SDA -> GPIO4
    2. SCL -> GPIO16
    3. OLED RESET -> GPIO16 -> in needs to be pulled LOW and then HIGH during OLED operation
3. VEML7700 runs as a separate task with a 4Hz frequency