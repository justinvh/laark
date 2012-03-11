# coding: utf-8
'''
good-features --
Reads a RAW BGR image via a PULL socket and uses the
OpenCV GoodFeaturesToTrack method to find subrectangles
to send over a PUSH socket.
'''

from cStringIO import StringIO
from laark.decorator.pipeline import pipeline
from itertools import izip, cycle
import argparse
import cv
import json
import os
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

    cv.SaveImage('/tmp/source.png', gs)

    bound = 512
    size = cv.GetSize(img)
    print '\nreceived %s image' % (size,)
    for ((x, y), i) in izip(cv.GoodFeaturesToTrack(gs, eigimg, tmpimg, args.num_features, 0.02, 1.0, useHarris=True), cycle(range(args.num_features))):
        x, y = int(x), int(y)
        # TODO(cwvh): This is fast, dirty, and crude. Moving by powers of two
        # could potentially throw away a lot of information.
        #
        # Take a subrectangle which is centered at (x,y) with a bounding
        # box which is (x±bound,y±bound). If any edge of the box is outside of
        # the original image, reduce the bounding box size by powers of two to 
        # create a tight bounding box.
        tightbound = bound
        minx, miny = x-bound, y-bound
        maxx, maxy = x+bound, y+bound
        while minx < 0 or miny < 0 or maxx > size[0] or maxy > [1]:
            # An edge exceeded the image bounds. Calculate new bounds with
            # a smaller bounding box (tightbound) and check the coordinates again.
            tightbound /= 2
            minx, miny = x-tightbound, y-tightbound
            maxx, maxy = x+tightbound, y+tightbound

        if tightbound < 8:
            print 'bounding-box less than one; skipping feature'
            continue

        bounds = (x, y, tightbound, tightbound)
        print '%d feature=%s,\tsubrect=%s' % (i, (x,y), bounds)
        sub = cv.GetSubRect(img, bounds)
        filename = '/tmp/feature-%d.png' % i
        # cv.SaveImage won't overwrite files. Do it the hard way.
        if os.path.exists(filename):
            os.unlink(filename)

    return "TODO: allow pipeline yields"

worker.run()
