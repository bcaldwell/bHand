import numpy as np
import cv2
import imutils
from imutils import contours
import audiogen

cap = cv2.VideoCapture(0)
# fgbg = cv2.bgsegm.createBackgroundSubtractorMOG()

def play(freq, time =0.5):
    puts freq
    n = audiogen.sampler.play(audiogen.tone(freq), blocking=False)
    time.sleep(time)
    n.close()

while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()

    # frame = fgbg.apply(frame)
    # Our operations on the frame come here
    b1,g1,r1 = cv2.split(frame)

    b1 = b1.astype(int)
    g1 = g1.astype(int)
    r1 = r1.astype(int)


    ts1 = np.maximum(np.zeros_like(b1),np.round(b1 - 0.4*r1 - 0.4*g1))

    t1 = ts1.astype('uint8')


    #find mean value and max value
    mean1 = t1.sum()/t1.size
    max1 = t1.max()

    #thresholding
    _,t1 = cv2.threshold(t1,50,255,cv2.THRESH_BINARY)

    cnts = cv2.findContours(t1.copy(), cv2.RETR_EXTERNAL,
	cv2.CHAIN_APPROX_SIMPLE)
    cnts = cnts[0] if imutils.is_cv2() else cnts[1]
    cnts = contours.sort_contours(cnts)[0]

    print cnts
    # loop over the contours
    for (i, c) in enumerate(cnts):
    	# draw the bright spot on the image
    	(x, y, w, h) = cv2.boundingRect(c)
    	((cX, cY), radius) = cv2.minEnclosingCircle(c)
    	cv2.circle(t1, (int(cX), int(cY)), int(radius),
    		(0, 0, 255), 3)
    	cv2.putText(t1, "#{}".format(i + 1), (x, y - 15),
    		cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 0, 255), 2)

        play(440 + x, 0.1)



    # Show keypoints


    # Display the resulting frame
    cv2.imshow('blue', t1)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()
