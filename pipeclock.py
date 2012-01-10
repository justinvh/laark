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
                    help='the ports that pipeclock will pull from')

args = parser.parse_args()

n = 0
start = last = time.time()
@pipeline(in_ports=args.in_ports)
def timer(_):
    global start, last, n
    current = time.time()
    print "%d %f %f" % (n, current-start, current-last)
    last = current
    n += 1

timer.run()
