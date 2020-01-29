from PIL import Image
import struct
import os

#Open blob file for QSPI operation
blob = open('unified.blob', "rb")


newFile = open("gif.raw", "wb")

data = blob.read(1)
while data:
    newFile.write(data)
    data = blob.read(1)

sorted_list = os.listdir("./tmp/")
sorted_list.sort()

count = 0;
for file in sorted_list:
    print(f"Decoding {file}")
    
    #Open and Decode image file
    im = Image.open(f"./tmp/{file}")
    width, height = im.size
    new_width = 0
    new_height = 0
    aspect_ratio = 480/272
    if (width / height < aspect_ratio):
        new_width  = 480
        new_height = int((480 / width) * height)
        im = im.resize((new_width, new_height), Image.ANTIALIAS)
        crop = int((new_height - 272) / 2)
        diff = 272 - (new_height - 2*crop)
        im = im.crop((0, crop, new_width, new_height - crop + diff))
        new_height = new_height - (2 * crop) + diff
    else:
        new_height = 272
        new_width  = int((272 / height) * width)
        im = im.resize((new_width, new_height), Image.ANTIALIAS)
        crop = int((new_width - 480) / 2)
        diff = 480 - (new_width - 2*crop)
        im = im.crop((crop, 0, new_width - crop + diff, new_height))
        new_width = new_width - (2 * crop) + diff
    
    print(f"New Resolution: {new_width} {new_height}")
    im.save(f'./output/test{count}.png')
    count = count + 1

    rgb_im = im.convert('RGB')
    
    for i in range(new_height):
        for j in range(new_width):
            red    = int(rgb_im.getpixel((j, i))[0])
            green   = int(rgb_im.getpixel((j, i))[1])
            blue  = int(rgb_im.getpixel((j, i))[2])

            value = int(blue/8) + int(green/4)*32 + int(red/8)*2048
            struct_encode = struct.pack('H', value)
            
            newFile.write(struct_encode)

blob.close()
newFile.close()