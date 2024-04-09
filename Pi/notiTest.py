from bluepy import btle
import time
import sys
import binascii
import struct
import csv

mac_ad = "0E:DF:9A:B2:12:94"
SERVICE_UUID = "181A"
CHARACTERISTIC_UUID = "2B44"
CHARACTERISTIC_UUID_2 = "2B9B"
CHARACTERISTIC_UUID_3 = "2A2C"

#trying to figure out how to handle data as soon as notification seen
#the arduino should 'notify' the characteristic whenever it updates the value
class MyDelegate(btle.DefaultDelegate):
    def __init__(self, handle):
        btle.DefaultDelegate.__init__(self)
        self.handle = handle
        
    def handleNotification(self, cHandle, data):
        print("A notification was received: %s" %data)
        vals = struct.unpack_from('<fff', data)
        print(vals)


print("Connecting...")
pls = True
while(pls):
    try:
        nano_ble = btle.Peripheral(mac_ad, addrType=btle.ADDR_TYPE_PUBLIC)
        pls = False
    except:
        print("Didn't work")

print("Discovering Services...")
_ = nano_ble.services
bleService = nano_ble.getServiceByUUID(SERVICE_UUID)

print("Discovering Characteristics...")
chOne = bleService.getCharacteristics()[0]
chTwo = bleService.getCharacteristics()[1]
chThree = bleService.getCharacteristics()[2]
print(chOne, chOne.propertiesToString())
print(chTwo, chTwo.propertiesToString())
print(chThree, chThree.propertiesToString())

nano_ble.setDelegate( MyDelegate(chOne.getHandle()) )
nano_ble.setDelegate( MyDelegate(chTwo.getHandle()) )
nano_ble.setDelegate( MyDelegate(chThree.getHandle()) )

descOne = chOne.getDescriptors()
descTwo = chTwo.getDescriptors()
descThree = chThree.getDescriptors()
print("desc", descOne)
print("desc", descTwo)
print("desc", descThree)

nano_ble.writeCharacteristic(descOne[0].handle, b"\x01\x00")
nano_ble.writeCharacteristic(descTwo[0].handle, b"\x01\x00")
nano_ble.writeCharacteristic(descThree[0].handle, b"\x01\x00")

while True:
    if nano_ble.waitForNotifications(0.2):
        continue

