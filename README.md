**NOTE:** The hardware is in its early development. This is meant to replace your garage opener, if you'd like to use this with your existing garage door opener only slight modifications are needed. As of now relays control the motor, in the future there will be a motor driver circuit integrated on the PCB.

# Firmware for esp-garage

ESP based garage opener using ESP RainMaker.

To build this you need esp-idf installed, I also recommend the Visual Studio Code extension.

Useful links:
- ESP RainMaker documentation [here](http://rainmaker.espressif.com/docs/get-started.html).
- ESP RainMaker C API [here](https://docs.espressif.com/projects/esp-rainmaker/en/latest/c-api-reference/index.html).

You can find the hardware [here](https://github.com/xhivo97/esp-garage-hardware)

## Prerequisites

Any of these:
- [esp-idf](https://github.com/espressif/esp-idf) installed and configured.
- [VS Code](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) with esp-idf extension installed and configured.
- [Eclipse](https://github.com/espressif/idf-eclipse-plugin) set-up with ESP-IDF.


## Setup with Visual Studio Code
- ### Clone repos
```
git clone  https://github.com/xhivo97/esp-garage-firmware.git
cd esp-garage-firmware
git clone --recursive https://github.com/espressif/esp-rainmaker.git
code .
```
- ### Build and flash
Run these with `F1` on VS Code:
- `ESP-IDF: Add vscode configuration folder`
- `ESP-IDF: Set Espressif device target`
- `ESP-IDF: Select port to use (COM, tty, usbserial)`
- `ESP-IDF: Build your project`
- `ESP-IDF: Flash (UART) your project`

## TO-DO List

- [x] Basic functionality.
    > ~~Open/Close function with relays.~~

    > ~~Alert user if door is open for X amount of time via a push notification.~~

    > ~~Functional when offline with the hardware button.~~

    > ~~On-board LEDs with an auto off timer.~~

    > ~~Motor runtime limit timer as a safety.~~

    > ~~Monitor door state via the app.~~

- [ ] Additional functionality.
    > Make a custom device/widget for the ESP RainMaker Android and iOS app that is better suited for a garage door.

    > Integrate with voice assistants.

_Will add more to the list as the PCB design progresses_
