# Motorloader

Tool to test generated load curves (python scripts) via an esp8266 as a digital servo tester (pwm input to esc).

Used for load testing of a fuel cell at TU Wien Space Team

## Features

- Control a servo motor using a web-based interface.
- Accept user inputs for servo control parameters via a web form.
- Process CSV data from the web form to adjust servo motor control.
- Toggle step read mode using a checkbox input.
    - This means, every 2nd item in the list is used as a delay (in ms)
- Option to comment `#define WEBSOCKET` to use this connection to relay additional data while running the simulation (progress etc)
- Most info is also written as serial output (just not the progress bar)

![Interface](https://i.ibb.co/bHBfbRf/image.png)

## Setup and Usage

1. Clone or download this repository.
2. Open the Arduino IDE and load the project's `.ino` file.
3. Make sure to install the necessary libraries (e.g., ESP8266WebServer, Servo) if not already installed.
4. Connect your ESP8266 board and upload the code to the board.
5. Access the web interface by entering the board's IP address or its hostname `motorloader/` in a web browser.
6. Follow the web interface instructions to control the servo motor an  d adjust parameters.
7. Use the example curves or adapt the python script templates to create your own csv input data. 
    - input should not end on a comma -> be sure to not copy the last "," from the saved files

- simply connect ground and signal cable from the ESP8266 to the ESC input
![Setup](https://i.ibb.co/CtkNmnY/20230823-194303.jpg)
## _wlan_credentials.h_ boilerplate
- you can use your mobile hotspot as a local network

```c
#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

const char* ssid = "motorloader_wlan"; // ssid
const char* password = "ml_pw321"; // password

#endif
```

## LED overview
- LED1: at antenna
    - on: ready to receive csv data
    - changing every 0.5s: trying to connect to wlan
    - blinking r in morse (.-.): connected to wlan/completed load run 
    - changing every 1s: running in simulation loop, just shows everything is working
- LED2: near USB port
    - on: everything OK
    - off: error processing CSV data
    - blinking: changes on/off every time a new value is given to the ESC

## possible ToDo
- additional error/ignore double/last comma in csv processing
- python example for discontinuous load curve
- maybe refactor time status to s instead of ms 
