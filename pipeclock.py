'''
pipeclock --
Reads from a pull socket and times how long each request takes. 
It does not process the messagse it receives.
'''

import argparse
import os
import time
import zmq

parser = argparse.ArgumentParser(
    description='Reads from a pull socket and prints out the message number, '
                'the time that it received thes message, and how long it '
                'took to receive it.')

parser.add_argument('--port', dest='port', type=int, nargs='?',
                    default=int(os.environ.get('PORT', 5558)),
                    help='the port that pipeclock will pull from')

args = parser.parse_args()

context = zmq.Context()
receiver = context.socket(zmq.PULL)
receiver.connect("tcp://localhost:%d" % args.port)

n = 0
start = last = time.time()
while True:
    receiver.recv()
    current = time.time()
    print "%d %f %f" % (n, current-start, current-last)
    last = current
    n += 1


