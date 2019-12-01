from PIL import Image
import struct

#Open and Decode image file
im = Image.open('mudkip.jpg')
width, height = im.size
rgb_im = im.convert('RGB')

#Open blob file for QSPI operation
blob = open('unified.blob', "rb")


newFile = open("mudkip.raw", "wb")

data = blob.read(1)
while data:
    newFile.write(data)
    data = blob.read(1)


for i in range(height):
    for j in range(width):
        red    = int(rgb_im.getpixel((j, i))[0])
        green   = int(rgb_im.getpixel((j, i))[1])
        blue  = int(rgb_im.getpixel((j, i))[2])

        value = int(blue/8) + int(green/4)*32 + int(red/8)*2048
        struct_encode = struct.pack('H', value)
        
        newFile.write(struct_encode)

blob.close()
newFile.close()