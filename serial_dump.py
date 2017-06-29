""" this program allows the user to communicate with Arduino via serial communitcation
There are 3 build in "functions"
1. send serial commands to arduino
2. Read serial commands from arduino
3. two way communitcation with arduino, using serial_commands running on arduino (in folder)

Please note pyserial is required can be downloaded from here (http://pyserial.sourceforge.net/)
also not that this program is written in python 3
"""


#import modules
import serial
import time
import os.path
import sys

#connect to arduino by trying the following list of serial ports, mac need to be added
locations=['/dev/ttyACM0','/dev/ttyACM1','/dev/ttyACM2','/dev/ttyACM3','/dev/ttyUSB0','/dev/ttyUSB1','/dev/ttyUSB2','/dev/ttyUSB3',
'/dev/ttyS0','/dev/ttyS1','/dev/ttyS2','/dev/ttyS3', '/dev/cu.usbmodem2852291', 'COM1','COM2','COM3', 'COM4']
connected =False #set connected to be false

#try each serial port, exit loop if one connects and set connected to True, else desplay failed to connect to any arduino
for device in locations:
	#try to connect to current device
    try:
        #print ("Trying..."+device)
        arduino = serial.Serial(device, 9600)
        connected=True
        break
    except:
        print ("Failed to connect on "+device)

if connected ==False:
    print ("\nFailed to connect on to any arduino")
    exit(1)
else:
	print ("Successfully connected to arduino\n")


#function converts assci to readable text
def convert (enter):
	out=""
	#See if input is integers representing ASCII values and convert if so
	try:
		for a in range (0,len(enter)):

			#if enter[a] is \n or 'enter' return string out
			if (enter[a])==13:
				return out
			else:	#if enter[a] is not \n then add the text value of it to out
				out=out+chr(enter[a])
	#Otherwise enter is a string, so make out == enter
	except TypeError:
		out = enter
	#if string is not returns and all characters have been used added, then return out
	return out

#function reads serial line from arduino and returns it.
def readline():
    reading = (arduino.readline())	#read line from arduino
    pr=convert (reading)	#convert to readable text
    return pr

if len(sys.argv) < 2:
    print("file name required")
    exit(1)

file_name = sys.argv[1]

if os.path.isfile(file_name):
    print("File already exists")
    file_name = f"{file_name} {time.time()}"

print (file_name)

f = open(file_name, 'w')

t = time.time()

while True:
    data = readline()
    data = f"{time.time() - t} {data}\n"
    # print(data)
    f.write(data)
