import os
import sys

#Parameters
blob_path = os.path.join(os.path.dirname(__file__),"unified.blob")

def pack_pic(source_file: str, dest_file: str):
	#Open files for reading
	image_file = open(source_file, "rb")
	blob_file  = open(blob_path, "rb")

	#Open output file for writing
	new_file   = open(dest_file, "wb")

	#Write blob file out
	data = blob_file.read(1)
	while data:
		new_file.write(data)
		data = blob_file.read(1)

	#Write image file out
	data = image_file.read(1)
	while data:
		new_file.write(data)
		data = image_file.read(1)

	#Close all files
	image_file.close()
	blob_file.close()
	new_file.close()


if __name__ == '__main__':
	if not (len(sys.argv) == 3):
		print("ERR: Incorrect number of arguments.\nUSAGE: python3 pad.py [input_file] [output_file]\n")
	else:
		pack_pic(sys.argv[1], sys.argv[2])
