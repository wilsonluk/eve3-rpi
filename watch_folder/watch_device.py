import sys
import os
import time
import subprocess
import base64

from pad import pack_data

#CONSTANTS
shutdown_file = 'shutdown.now'
lcd_main_folder = "/home/pi/EVE3-BT81x-Flash/"


def watch_folder(device: str, interval):
	old_data = None
	file = None
	while True:
		try:
			file = open(device, 'rb')
			data = file.read()
			if not (data == old_data):
				print("Found New Data:\n\n\n\n\n\n\n")
				print(data)
				pack_data(base64.b64decode(data), f'{lcd_main_folder}watch_folder/tmp.raw')
				start_lcd('jpg')
				data = old_data
		except FileNotFoundError:
			pass
		finally:
			if file is not None:
				file.close()
			time.sleep(interval)

def start_lcd(format: str):
	subprocess.call(['sudo', 'killall', 'lcd'])
	subprocess.Popen(['sudo', f'{lcd_main_folder}build/lcd', f'{lcd_main_folder}watch_folder/tmp.raw', format])

if __name__ == '__main__':
	#watch_folder [path] [interval]
	watch_folder('/dev/rfcomm0', 0.5)
