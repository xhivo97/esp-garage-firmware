![.github/workflows/build.yml](https://github.com/xhivo97/esp-garage-firmware/actions/workflows/build.yml/badge.svg)
# Features
- Controls the motor direction with two relays.
- Controls a light via a relay.
- ESP Rainmaker iOS and Android app.
- Live updates of the door status on the phone app.
- Configurable motor runtime safety limiter.
- Detects possible limit switch faults.
- Sends push notification if door is open for more than 15 minutes.
- Events logged and viewable in ESP Rainmaker's dashboard.

## Prerequisites
Any of these:
- [esp-idf](https://github.com/espressif/esp-idf) installed and configured.
- [VS Code](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) with esp-idf extension installed and configured.
- [Eclipse](https://github.com/espressif/idf-eclipse-plugin) set-up with ESP-IDF.

# Building and flashing
## Clone repos
```
git clone  https://github.com/xhivo97/esp-garage-firmware.git
cd esp-garage-firmware
git clone --recursive https://github.com/espressif/esp-rainmaker.git
```
## Using Visual Studio Code
Run these commands with `F1`:
- `ESP-IDF: Add vscode configuration folder`
- `ESP-IDF: Set Espressif device target`
- `ESP-IDF: Select port to use (COM, tty, usbserial)`
- `ESP-IDF: Build your project`
- `ESP-IDF: Flash (UART) your project`

## Using esp-idf
For this example I'm using the `esp32-c3`

You can list the available boards with `idf.py --list-targets`
```
idf.py set-target esp32-c3
idf.py build
idf.py flash
```

# Useful links
### ESP RainMaker documentation [here](http://rainmaker.espressif.com/docs/get-started.html).
### ESP RainMaker C API [here](https://docs.espressif.com/projects/esp-rainmaker/en/latest/c-api-reference/index.html).
### Garage Opener PCB files [here](https://github.com/xhivo97/esp-garage-hardware)
