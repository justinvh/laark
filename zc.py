'''
zmqcat -- 
reads from standard input until it gets killed
sends the result over a push socket on port 5558 or the port defined by the
environmental variable PORT
'''

import os
import sys
import zmq

port = os.environ.get('PORT', '5558')

context = zmq.Context()
sender = context.socket(zmq.PUSH)
sender.bind("tcp://*:" + port)

while True:
    for line in sys.stdin:
        sender.send(line)

