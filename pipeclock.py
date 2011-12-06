'''
pipeclock --
reads from a pull socket and times how long each request takes, ignores what it
receives
'''

import os
import time
import zmq

port = os.environ.get('PORT', '5558')

context = zmq.Context()
receiver = context.socket(zmq.PULL)
receiver.connect("tcp://localhost:" + port)

requests = 0
start = time.time()
while True:
    receiver.recv()
    current = time.time()
    print "Request %d at time %f" % (requests, (current-start))
    requests += 1


