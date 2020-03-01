import sys
import os
import time
import subprocess

from pad import pack_pic

#CONSTANTS
shutdown_file = 'shutdown.now'
lcd_main_folder = "/home/pi/EVE3-BT81x-Flash/"

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
				start_lcd('gif')
				time.sleep(5)
				subprocess.call(['sudo', 'poweroff'])

			#Display JPEG on screen
			if (extension in ['.jpeg', '.jpg']):
				pack_pic(path + filename, f'{lcd_main_folder}watch_folder/tmp.raw')
				start_lcd('jpg')

			#Display RGB image on screen
			elif (extension in ['.rgb']):
				pack_pic(path + filename, f'{lcd_main_folder}watch_folder/tmp.raw')
				start_lcd('rgb')

			#Animate GIF on screen
			elif (extension in ['.gif', '.jif']):
				subprocess.call(['sudo', 'killall', 'lcd'])
				tmp = ['ffmpeg', '-y', '-i', path + filename, '-c:v', 'mjpeg', '-q:v', '5', '-vf', 'fps=fps=30,scale=480:272:force_original_aspect_ratio=increase,crop=480:272', '-pix_fmt', 'yuvj420p', '-an', f'{lcd_main_folder}watch_folder/tmp.avi']
				print(" ".join(tmp))
				subprocess.call(tmp, stdout=subprocess.PIPE)
				pack_pic(f'{lcd_main_folder}watch_folder/tmp.avi', f'{lcd_main_folder}watch_folder/tmp.raw')
				#os.remove(f'{lcd_main_folder}watch_folder/tmp.avi')
				start_lcd('gif')

			os.remove(path + filename)
		else:
			#print("WAIT")
			time.sleep(interval)

def start_lcd(format: str):
	subprocess.call(['sudo', 'killall', 'lcd'])
	subprocess.Popen(['sudo', f'{lcd_main_folder}build/lcd', f'{lcd_main_folder}watch_folder/tmp.raw', format], stdout=subprocess.PIPE)

if __name__ == '__main__':
	#watch_folder [path] [interval]
	watch_folder('/home/pi/Downloads/', ['.jpg', '.gif', '.jif'], 0.5)
