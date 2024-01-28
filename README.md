
# Sniffy

## Introduction
Welcome to Sniffy, a project developed for learning and experimentation with the ESP32-C3 microcontroller. Sniffy showcases the ability to generate multiple fake Wi-Fi Access Points (APs) and sniff out nearby Wi-Fi networks and devices. It's a playful tool designed for those interested in understanding wireless network interactions.

**Disclaimer:** This project is intended for educational purposes only. Unauthorized or malicious use is strictly prohibited.

## Features
- **Fake AP Generation:** Generate a number of fake Wi-Fi access points.
- **Wi-Fi Sniffing:** Detect and list nearby Wi-Fi access points and devices.
- **Deauther Capabilities (Non-Functional):** An attempt was made to include deauther functionalities. However, due to hardware limitations of the ESP32-C3, this feature does not work. It is noted that ESP8266 boards might support these capabilities, but this code is not compatible with ESP8266.

## Hardware Requirements
- ESP32-C3 microcontroller

## Software Requirements
- For detailed instructions on setting up the development environment, refer to Espressif's official guide for [VS Code setup for ESP32](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/get-started/vscode-setup.html).

## Installation
1. Clone or download the Sniffy source code.
2. Set up your development environment following the instructions provided in the [Espressif guide](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/get-started/vscode-setup.html).
3. Open the project in your configured development environment.
4. Connect your ESP32-C3 to your computer.
5. Compile and upload the code to your ESP32-C3.

## Contributing
I welcome contributions to Sniffy. Feel free to fork the repository, make your changes, and submit a pull request. For bugs and feature requests, please open an issue in the repository.

## Ethical Use and Disclaimer
This project is for educational purposes only. The developers of Sniffy do not endorse and are not responsible for misuse of the software. Users must comply with all applicable local laws and regulations.
