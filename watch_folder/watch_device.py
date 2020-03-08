import sys
import os
import time
import subprocess
import base64
import binascii

from pad import pack_data

#CONSTANTS
lcd_main_folder = "/home/pi/EVE3-BT81x-Flash/"


def watch_folder(device: str, interval):
	old_data = None
	file = None
	while True:
		try:
			file = open(device, 'rb')
			data = file.read()
			if not (data == old_data):
				print("\nNew Data Received!")
				handle_data(data)
				#control_code = data[:2]
				#print(control_code)
				#pack_data(base64.b64decode(data), f'{lcd_main_folder}watch_folder/tmp.raw')
				#start_lcd('jpg')
				data = old_data
		except FileNotFoundError:
			pass
		finally:
			if file is not None:
				file.close()
			time.sleep(interval)

def handle_data(data):
	#print(f'DATA:    {binascii.hexlify(data[:2])}')
	control_code = int.from_bytes(data[:2], byteorder = 'big')
	print(f"Control Code {control_code} Found")
	if (control_code == 0):
		#print(data)
		print("Sending data to screen")
		pack_data(base64.b64decode(data[1:]), f'{lcd_main_folder}watch_folder/tmp.raw')
		start_lcd(0, None)
	elif (control_code == 1):
		new_brightness = int.from_bytes(data[2:4], byteorder = 'big')
		print(f"Received Brightness: {new_brightness}")
		start_lcd(1, new_brightness)
	elif (control_code == 255):
		print("Shutdown code received, pretend the Pi is shutting down")
	else:
		print("WARN: Control code not recognized, assuming JPEG image")
		pack_data(base64.b64decode(data), f'{lcd_main_folder}watch_folder/tmp.raw')
		start_lcd(0, None)

def start_lcd(format: str, params):
	subprocess.call(['sudo', 'killall', 'lcd'])
	if (format == 0):
		subprocess.Popen(['sudo', f'{lcd_main_folder}build/lcd', f'{lcd_main_folder}watch_folder/tmp.raw', 'jpg'])
	elif (format == 1):
		subprocess.Popen(['sudo', f'{lcd_main_folder}build/lcd', str(params), 'brightness'])

if __name__ == '__main__':
	#watch_folder [path] [interval]
	watch_folder('/dev/rfcomm0', 0.5)
