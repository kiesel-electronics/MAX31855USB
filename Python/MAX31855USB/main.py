import time
import pymeasure
from MAX31855USB import MAX31855USB
from MAX31855USB import list_resources

print("Installed pymeasure version: " + pymeasure.__version__)
print("Available resources:")
print(list_resources())

tc = MAX31855USB('ASRL13')
print("Device id: " + tc.id())

for i in range(500):
    print("Measured temperature (°C):          " + str(tc.get_temperature()))
    print("Measured internal temperature (°C): " + str(tc.get_internal_temperature()))
    time.sleep(1)
