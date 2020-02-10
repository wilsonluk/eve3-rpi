from PIL import Image
import os

#Open blob file for QSPI operation
blob = open('/home/pi/EVE3-BT81x-Flash/converter/unified.blob', "rb")

#Open binary fiel to be written to
newFile = open("/home/pi/EVE3-BT81x-Flash/images/corgi.raw", "wb")

data = blob.read(1)
while data:
    newFile.write(data)
    data = blob.read(1)

jpeg = open("/home/pi/EVE3-BT81x-Flash/images/corgi.avi", "rb")
data = jpeg.read(1)
while data:
    newFile.write(data)
    data = jpeg.read(1)

jpeg.close()
blob.close()
newFile.close()