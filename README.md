# OpenSportSense Reaction Pods

**Open-source, ESP32-powered reaction training pods**

Build an interactive reaction training system using an ESP32, four wired illuminated push buttons, 3D-printed parts, and a browser-based dashboard. This project is designed for makers, educators, coaches, and anyone interested in sports, physical computing, and interactive learning.

> **Current build:** Version 1 (working prototype). Documentation and designs are being improved over time.

<!-- Add a photo of the completed pods here:
![Four reaction pods](images/reaction-pods.jpg)
-->

## How it works

1. **Physical pods:** Four wired targets each contain a tactile push button and individually addressable RGB LEDs.
2. **ESP32 controller:** Arduino firmware drives the LEDs, reads button presses, manages game timing, and records performance.
3. **Local browser dashboard:** The ESP32 creates a Wi-Fi access point and serves the game's HTML interface. Connect a phone, tablet, or computer to the ESP32 Wi-Fi network to control the game without a cloud service or subscription.

In the current reaction-time game, one target lights green at a time. Press the illuminated pod to register a hit. The dashboard displays reaction-time and gameplay metrics. Timing, LEDs, and game rules can be changed in the firmware.

## Find the project files

| Location | What goes here |
| --- | --- |
| [Version 1](versions/v1/) | Version-specific overview |
| [ESP32 firmware](versions/v1/firmware/) | Arduino sketch and libraries/setup notes |
| [Web interface](versions/v1/interface/) | Readable HTML/CSS/JavaScript source hosted by the ESP32 (may be embedded in the firmware) |
| [3D-print files](versions/v1/stl/) | STL models for printable pod components |
| [Build guide](versions/v1/BUILD_GUIDE.md) | Step-by-step assembly and setup instructions (in progress) |
| [Images](images/) | Project photos, wiring diagrams, and screenshots |

Future designs will go into `versions/v2/`, `versions/v3/`, and so on, keeping Version 1 reproducible.

## Parts and supplies — Version 1

These are **starting-point supplier links**, not a final verified bill of materials. Check component dimensions, current requirements, and the final wiring guide before purchasing.

| Part | Quantity | Link / notes |
| --- | --- | --- |
| ESP32 development board | 1 | [Espressif ESP32-DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc) (check board/pin compatibility) |
| 12 × 12 × 7.3 mm tactile push buttons | 4 | [Search for matching switches](https://www.google.com/search?q=12x12x7.3mm+tactile+push+button) |
| WS2812 / NeoPixel-compatible addressable RGB LEDs | For 4 pods | [Adafruit NeoPixel guide](https://learn.adafruit.com/adafruit-neopixel-uberguide) (verify LED count and voltage) |
| Breadboard or prototyping board | 1 | [Breadboards](https://www.adafruit.com/category/124) or [Perma-Proto board](https://www.adafruit.com/product/590) |
| Hookup wire / jumper wires | As needed | [Wires and cables](https://www.adafruit.com/category/82) |
| USB data cable | 1 | Match the ESP32's connector |
| LED-compatible power source | 1 | Exact rating to be confirmed in the build guide |
| 3D-printed parts | 4 sets | Files will be posted in [stl](versions/v1/stl/) |
| 6 mm plywood or other rigid stand material | Optional | Supports the button; alternatives are fine |
| Heat-set threaded inserts and matching screws | Optional | Depends on assembly design |
| Soldering iron, solder, wire stripper | Optional / recommended | Useful for durable connections |

**Software:** [Arduino IDE](https://www.arduino.cc/en/software), [Arduino ESP32 board support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html), and the [Adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel).

## Quick start

1. Gather the parts listed above.
2. Download the printable models from [`versions/v1/stl/`](versions/v1/stl/) once uploaded, and print the four pod assemblies.
3. Assemble and wire the buttons and addressable LEDs to the ESP32 according to the [build guide](versions/v1/BUILD_GUIDE.md).
4. Open the Arduino sketch from [`versions/v1/firmware/`](versions/v1/firmware/) in Arduino IDE, install its dependencies, and upload it.
5. Connect your device to the ESP32's Wi-Fi network and open the local dashboard. The exact network name, address, and troubleshooting details will be documented with the firmware.

**Note:** Firmware, STLs, photos, and a full verified wiring/power guide still need to be uploaded. This README is the project starting point, not yet a complete build package.

## Remix and contribute

Change LED patterns, reaction timing, game modes, dashboards, or pod enclosures to fit your own use case. Contributions and documented improvements are welcome.

## About

OpenSportSense makes tools that help people measure, understand, and improve how they play or train.

This is an independent open-source project and is not affiliated with or endorsed by BlazePod.

**License:** To be selected. Please do not assume reuse permissions until a license file is added.
