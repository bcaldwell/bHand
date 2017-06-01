# import the necessary packages
from collections import deque
import numpy as np
import argparse
import imutils
import cv2
import audiogen
import time

import sys

note_being_played = None

notes = [
    [440, "A"],
    # [366.16, "A#"],
    [493.88, "B"],
    [523.25, "C"],
    # [554.37, "C#"],
    [587.33,"D"],
    # [622.25, "D#"],
    [659.25, "E"],
    [698.46, "F"],
    # [739.99, "F#"],
    [783.99, "G"],
    # [830.61, "G#"]
]

def play(freq, t = 0.01):
    print str(freq) + " " + str(t)
    global note_being_played
    n = audiogen.sampler.play(audiogen.tone(freq), blocking=False)
    time.sleep(0.1)
    if note_being_played is not None:
        note_being_played.close()
    time.sleep(t)
    note_being_played = n

def close ():
    global note_being_played
    if note_being_played is not None:
        note_being_played.close()

# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video",
    help="path to the (optional) video file")
ap.add_argument("-b", "--buffer", type=int, default=64,
    help="max buffer size")
args = vars(ap.parse_args())

# define the lower and upper boundaries of the "green"
# ball in the HSV color space, then initialize the
# list of tracked points

#greenLower = (150, 150, 180)
#greenUpper = (255, 220, 230)
greenLower = (130, 120, 120)
greenUpper = (210, 200, 200)

pts = deque(maxlen=args["buffer"])

# if a video path was not supplied, grab the reference
# to the webcam
if not args.get("video", False):
    camera = cv2.VideoCapture(0)

# otherwise, grab a reference to the video file
else:
    camera = cv2.VideoCapture(args["video"])

    # keep looping
while True:
    # grab the current frame
    (grabbed, frame) = camera.read()

    # if we are viewing a video and we did not grab a frame,
    # then we have reached the end of the video
    if args.get("video") and not grabbed:
        break

    # resize the frame, blur it, and convert it to the HSV
    # color space
    width = 600
    # frame.shape[:2][1]
    frame = imutils.resize(frame, width=width)
    # blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # construct a mask for the color "green", then perform
    # a series of dilations and erosions to remove any small
    # blobs left in the mask
    mask = cv2.inRange(hsv, greenLower, greenUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)
    # find contours in the mask and initialize the current
    # (x, y) center of the ball
    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
        cv2.CHAIN_APPROX_SIMPLE)[-2]
    center = None

    # only proceed if at least one contour was found
    if len(cnts) > 0:
        # find the largest contour in the mask, then use
        # it to compute the minimum enclosing circle and
        # centroid
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        # only proceed if the radius meets a minimum size
        if radius > 10:
            # draw the circle and centroid on the frame,
            # then update the list of tracked points
            cv2.circle(frame, (int(x), int(y)), int(radius),
                (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)

    # update the points queue
    pts.appendleft(center)

    # loop over the set of tracked points
    # for i in xrange(1, len(pts)):
        # if either of the tracked points are None, ignore
        # them
        # if pts[i - 1] is None or pts[i] is None:
            # continue

        # otherwise, compute the thickness of the line and
        # draw the connecting lines
        # thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
        # cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)

    # show the frame to our screen
    #cv2.imshow("Frame", hsv)

    if pts[0] is not None:
        # print(len(pts))
        print(pts[0][0])
        x = width - pts[0][0]
        note = notes[int((x / float(width)) * (len(notes) * 1.0))]
        print(note)
        cv2.putText(frame, note[1], (50,width // 2), cv2.FONT_HERSHEY_SIMPLEX, 4, (0,255,255), 4)
        play(note[0])
    else:
        close()

    cv2.imshow("Frame", frame)

    key = cv2.waitKey(1) & 0xFF

    # if the 'q' key is pressed, stop the loop
    if key == ord("q"):
        break

# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()
