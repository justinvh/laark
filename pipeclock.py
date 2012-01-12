'''
pipeclock --
Reads from a pull socket and times how long each request takes. 
It does not process the messages it receives.
'''

import argparse
import os
import time
from pipeline import pipeline

parser = argparse.ArgumentParser(
    description='Reads from pull socket(s) and prints out the message number, '
                'the time that it received the message, and how long it '
                'took to receive it.')

parser.add_argument('--in-ports', dest='in_ports', type=int, nargs='+',
                    default=[5999],
                    help='The ports that pipeclock will pull from.')

parser.add_argument('--out-port', dest='out_port', type=int, nargs='?',
                    default=6000,
                    help='The port that pipeclock will forward data to.')

parser.add_argument('--forward', dest='forward', type=bool, nargs='?',
                    default=False,
                    help='Whether data should be forwarded.')

args = parser.parse_args()

n = 0
start = last = time.time()
def timer(data=None):
    global start, last, n
    current = time.time()
    print "%d %f %f" % (n, current-start, current-last)
    last = current
    n += 1
    return data

@pipeline(in_ports=args.in_ports, out_port=args.out_port)
def timer_forward(data):
    return timer(data)

@pipeline(in_ports=args.in_ports)
def timer_no_forward(_):
    timer()

if args.forward in (None, True):
    timer_forward.run()
else:
    timer_no_forward.run()
