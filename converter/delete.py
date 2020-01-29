import os
from shutil import copyfile

max_frames = 16
tmp_dir    = './tmp/'

#Clear tmp directory
for file in os.listdir('./tmp/'):
    os.remove(f"./tmp/{file}")

#Find number of files and determine offsets
num_frames = len(os.listdir('./sun/')) - 1
frame_skip = num_frames / max_frames
if (frame_skip >= 1):
    if (int(frame_skip) != frame_skip):
        frame_skip = int(frame_skip) + 1
else:
    frame_skip = 1
    
print(f"Keeping every {frame_skip} frames")

#Copy selected files to the tmp directory
source_counter = 0
counter = 0
while (source_counter <= num_frames):
    print(f"Copying ./sun/frame_{str(source_counter).zfill(2)}_delay-0.04s.jpg")
    copyfile(f"./sun/frame_{str(source_counter).zfill(2)}_delay-0.04s.jpg", f"./tmp/{str(counter).zfill(2)}.jpg")
    source_counter = source_counter + frame_skip
    counter = counter + 1
    
    