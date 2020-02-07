import struct

#Open and Decode image file
mjpeg = open('/home/pi/EVE3-BT81x-Flash/converter/sun.avi', "rb")

#Open blob file for QSPI operation
blob = open('/home/pi/EVE3-BT81x-Flash/converter/unified.blob', "rb")


newFile = open("/home/pi/EVE3-BT81x-Flash/converter/sun.raw", "wb")

data = blob.read(1)
while data:
    newFile.write(data)
    data = blob.read(1)

data = mjpeg.read(1)
while data:
    newFile.write(data)
    data = mjpeg.read(1)

mjpeg.close()
blob.close()
newFile.close()