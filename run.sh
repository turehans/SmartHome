arduino-cli compile --fqbn esp32:esp32:esp32 SmartHome.ino
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 SmartHome.ino
arduino-cli monitor -p /dev/ttyUSB0 -b esp32:esp32:esp32 -c baudrate=115200
