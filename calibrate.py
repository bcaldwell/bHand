import cv2
import numpy as np



camera = cv2.VideoCapture(0)
for i in range (10):
    (grabbed, frame) = camera.read()

hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

def print_color(event,x,y,flags,param):
    # print len(hsv)
    if event == 4:
        print hsv[y][x]

cv2.imshow("Frame", hsv)
cv2.setMouseCallback('Frame',print_color)
key = cv2.waitKey(0)

camera.release()
