'''
good-features --
Reads a RAW BGR image via a PULL socket and uses the
OpenCV GoodFeaturesToTrack method to find subrectangles
to send over a PUSH socket.
'''

from cStringIO import StringIO
from laark.decorator.pipeline import pipeline
import argparse
import cv
import json
import sys
import struct

parser = argparse.ArgumentParser(
        description='Reads a RAW BGR image and finds '
                    'interesting subrects to forward')

parser.add_argument('-n', '--num-features', type=int, default=10, nargs=1,
        help='The number of features to extract from the incoming image')

parser.add_argument('-i', '--in-port', type=int,
        help='The incoming port to listen for RAW images')

parser.add_argument('-o', '--out-port', type=int, nargs='?',
        help='The port to PUSH subrectangles out')

args = parser.parse_args()
print 'listening on', args.in_port, 'and pushing to', args.out_port, '...'

@pipeline(in_ports=args.in_port, out_port=args.out_port)
def worker(data):
    print "Reading..."
    buf = StringIO(data)
    raw = buf.read()

    header_size = struct.unpack('<I', raw[:4])[0]
    print header_size
    meta_raw = raw[4:header_size+4]
    print meta_raw
    meta = json.loads(meta_raw)
    width = meta['width']
    height = meta['height']
    channels = meta['channels']

    print 'Header Size: %d\nWidth: %d\nHeight: %d\nChannels: %d\n' % (
        header_size, width, height, channels)

    img = cv.CreateImageHeader((width, height), cv.IPL_DEPTH_8U, channels)
    cv.SetData(img, raw[header_size+4:])
    gs = cv.CreateImage((width, height), 8, 1)
    cv.CvtColor(img, gs, cv.CV_BGR2GRAY)
    
    eigimg = cv.CreateMat(gs.width, gs.height, cv.CV_8UC1)
    tmpimg = cv.CreateMat(gs.width, gs.height, cv.CV_8UC1)

    cv.SaveImage('/tmp/source.jpg', gs)

    print '\nnew image'
    for i, (x, y) in enumerate(cv.GoodFeaturesToTrack(gs, eigimg, tmpimg, int(args.num_features[0]), 0.05, 1.0, useHarris=True)):
        print 'good feature at', x, y
        x, y = int(x), int(y)
        cwidth, cheight = 128, 128
        sub = cv.GetSubRect(img, (x, y, cwidth, cheight))

        filename = '/tmp/feature-%d.png' % i
        cv.SaveImage(filename, sub)

    return "asdf"

worker.run()
