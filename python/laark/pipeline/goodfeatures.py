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

parser = argparse.ArgumentParser(
        description='Reads a RAW BGR image and finds '
                    'interesting subrects to forward')

parser.add_argument('-n', '--num-features', type=int, default=10, nargs='?',
        help='The number of features to extract from the incoming image')

parser.add_argument('-i', '--in-port', type=int,
        help='The incoming port to listen for RAW images')

parser.add_argument('-o', '--out-port', type=int, nargs='?',
        help='The port to PUSH subrectangles out')

args = parser.parse_args()
print 'listening on', args.in_port, 'and pushing to', args.out_port, '...'

@pipeline(in_ports=args.in_port, out_port=args.out_port)
def worker(data):
    buf = StringIO(data)
    raw = buf.read()

    # 4-byte integer: 4294967295
    header_size = int(raw[:10].lstrip())
    meta = json.loads(raw[10:10+header_size])
    width = meta['width']
    height = meta['height']
    channels = meta['channels']

    img = cv.CreateImageHeader((width, height), cv.IPL_DEPTH_8U, channels)
    cv.SetData(img, raw[10+header_size:])
    gs = cv.CreateImage((width, height), 8, 1)
    cv.CvtColor(img, gs, cv.CV_BGR2GRAY)
    
    eigimg = cv.CreateMat(gs.width, gs.height, cv.CV_8UC1)
    tmpimg = cv.CreateMat(gs.width, gs.height, cv.CV_8UC1)

    print '\nnew image'
    for x, y in cv.GoodFeaturesToTrack(gs, eigimg, tmpimg, args.num_features, 0.02, 1.0, useHarris=True):
        print 'good feature at', x, y
        sub = cv.GetSubRect(img, (int(x), int(y), 128, 128))

        filename = '/tmp/feature-%d-%d.png' % (x, y)
        cv.SaveImage(filename, sub)

    return "asdf"

worker.run()
