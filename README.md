# Motorloader

Tool to test generated load curves (python scripts) via an esp8266 as a digital servo tester (pwm input to esc).

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
5. Access the web interface by entering the board's IP address in a web browser.
6. Follow the web interface instructions to control the servo motor and adjust parameters.
7. Use the example curves or adapt the templates to create your own csv input data.
