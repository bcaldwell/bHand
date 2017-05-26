import audiogen
import itertools
import sys
import time

#beats = audiogen.mixer(
#        (audiogen.tone(440), audiogen.tone(445)),
#        [(constant(1), constant(1)),]
#)

def play(freq, time =0.5):
  n = audiogen.sampler.play(audiogen.tone(freq), blocking=False)
  time.sleep(time)
  n.close()

play(440)
play(500)
