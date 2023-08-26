# Motorloader

Tool to test generated load curves (python scripts) via an esp8266 as a digital servo tester (pwm input to esc).
Used for load testing of a fuel cell @ TU Wien Space Team

## Features

- Control a servo motor using a web-based interface.
- Accept user inputs for servo control parameters via a web form.
- Process CSV data from the web form to adjust servo motor control.
- Toggle step read mode using a checkbox input.
    - This means, every 2nd item in the list is used as a delay (in ms)

## Setup and Usage

1. Clone or download this repository.
2. Open the Arduino IDE and load the project's `.ino` file.
3. Make sure to install the necessary libraries (e.g., ESP8266WebServer, Servo) if not already installed.
4. Connect your ESP8266 board and upload the code to the board.
5. Access the web interface by entering the board's IP address or its hostname `motorloader/` in a web browser.
6. Follow the web interface instructions to control the servo motor and adjust parameters.
7. Use the example curves or adapt the templates to create your own csv input data.
    - input should not end on a comma -> be sure to not copy the last "," from the saved files

## _wlan_credentials.h_ boilerplate

```c
#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

const char* ssid = "motorloader_wlan"; // ssid
const char* password = "ml_pw321"; // password

#endif
```

## ToDo
- last step of progress bar gone ?!
- make csv processing with socket?
- test without socket