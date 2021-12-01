# MAX31855USB

This is the code repository for the 4 channel MAX31855 USB K-Type Thermocouple Sensor Board.
It contains the firmware code for the microcontroller as well as the python examples for reading the sensors.
The Board can either be used for data acquisition via USB using a virtual serial port or as a standalone data logger recording 
data onto a micro SD card.  
The Board can be extended with up to 8 MCP9808 precision temperature sensors.

![MAX31855USB](./img/1.jpg) 
![MAX31855USB](./img/2.jpg)


# Data logging
Standalone data logging requires a micro USB card formatted with FAT16 or FAT32.  
The logging configuration needs to be written as config.txt directly in the root folder of the SD card.  
A template can be found in this repository.

There are three parameters in the config file.
* csv_delimiter: defines the character separating the entries
* decimal_separator: usually `.` or `,`
* logging_interval: logging interval in seconds

Write the config file to the root folder of the SD card. Insert the SD cart into the slot of the board.  
Once powered up, the data logging starts when the start/stop button gets pressed.   
The LED blinks briefly each time, when a new data point is recorded.  
Another button press stops the data logging.  

# USB data acquisition
The sensors can be read using SCPI style commands.  
The following commands are supported:
* `*IDN?` Returns the device id string, something like "MAX31855USB,v1.0"
* `:TEMPerature?` Returns the temperature of the connected sensors. The values are separated by `,`.
* `:TEMPerature:INTern?` Returns the internal temperature of the MAX31855 chips. The values are separated by `,`.

A python library for data acquisition and an example how to read the sensors can be found in the python folder of this repository. 
