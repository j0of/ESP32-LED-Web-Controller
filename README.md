# ESP32 LED Web Controller
## A simple web controller for IKEA RGB LED strips on ESP32 and PlatformIO

### Demo
Enjoy this crappy GIF showing how the project works.
![demo](https://github.com/user-attachments/assets/5e998ddd-a19d-4d09-a147-d736357edda7)


### Requirements
- ESP32 denky/WROOM32
- Jumpers (4)
- 5V IKEA RGB LED strip (I have [these](https://www.ikea.com/gb/en/p/vattensten-led-lighting-strip-multicolour-40530597/))
  - Must have 4 pins: 5V; Green; Blue; Red
- VSCode PlatformIO extension

### Usage
- Clone repo and edit the [src/main.cpp](https://github.com/j0of/ESP32-LED-Web-Controller/blob/ed80f2e9092a5800859dc90290cb40e50e5aa9be/src/main.cpp#L12) file on lines 12 and 13 to have your own WiFi SSID and password
- Upload code to ESP32 using PlatformIO
- Use jumpers to connect corresponding GPIO pins to LED pins
- Access controller in browser by entering ESP32's local IP (displayed in Serial monitor during startup)

