import sys
import os
import time
import subprocess


from pad import pack_pic

#CONSTANTS
shutdown_file = 'shutdown.now'

#Scan for filetypes in a path
def scan_folder(path: str, file_types: list):
	files = os.listdir(path)
	for file in files:
		extension = os.path.splitext(file)[1]
		if (extension in file_types) or (file == 'shutdown.now'):
			return (file, extension)
	return None

def watch_folder(path: str, file_types: list, interval):
	while True:
		image_name = scan_folder(path, file_types)
		if not (image_name == None):
			filename, extension = image_name
			print(f'Found {filename}')

			#Shutdown if magic file sent
			if (filename == shutdown_file):
				os.remove(path + filename)
				print("WARN: SHUTDOWN SIGNAL RECEIVED")
				time.sleep(1)
				subprocess.call(['sudo', 'poweroff'])

			#Display JPEG on screen
			if (extension in ['.jpeg', '.jpg']):
				pack_pic(path + filename, 'tmp.raw')
				start_lcd('jpg')

			#Display RGB image on screen
			elif (extension in ['.rgb'])
				pack_pic(path + filename, 'tmp.raw')
				start_lcd('rgb')

			#Animate GIF on screen
			elif (extension in ['.gif', '.jif']):
				pass
				#TODO

			os.remove(path + filename

		else:
			print("WAIT")
			time.sleep(interval)

def start_lcd(format: str):
	subprocess.call(['sudo', '/home/pi/EVE3-BT81x-Flash/build/lcd', '/home/pi/EVE3-BT81x-Flash/watch_folder/tmp.raw'])

if __name__ == '__main__':
	#watch_folder [path] [interval]
	watch_folder('/home/pi/Downloads/', ['.jpg'], 0.5)
