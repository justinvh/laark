'''
zmqcat -- 
reads from standard input until it gets killed
sends the result over a push socket on port 5558 or the port defined by the
environmental variable PORT
'''

import argparse
import os
import sys
import zmq

parser = argparse.ArgumentParser(
    description='Read from stdin and send contents to a ZMQ socket')

parser.add_argument('--port', dest='port', type=int, nargs='?',
                    default=int(os.environ.get('PORT', 5558)),
                    help='the port that zc will push over')

args = parser.parse_args()

context = zmq.Context()
sender = context.socket(zmq.PUSH)
sender.bind("tcp://*:%d" % args.port)

while True:
    sender.send(sys.stdin.read())
    

