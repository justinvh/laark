#!/usr/bin/env python

'''
file-sink --
Reads from a PULL socket and dumps stream of raw bytes as the
specified file-format. The list of supported formats are given in
cv.LoadImage and is determined by file suffix. The default file-format
is PNG.

Common supported file suffixes: png, jpeg, jpg, tiff, tif
'''

from cStringIO import StringIO
from laark.decorator.pipeline import pipeline
import argparse 
import cv

parser = argparse.ArgumentParser(
        description='Reads from a PULL socket and dumps stream '
                    'as the specified file-format.')

parser.add_argument('-s', '--size', type=int, nargs=2,
        metavar = ('width', 'height'),
        help='The <width> <height> of the incoming stream.')

parser.add_argument('-f', '--filepath', default='/tmp/images/file-sink%d.png',
        nargs='?', help='The path and filename to save to. See cv.LoadImage '
                        'for the entire list of supported file formats.')

parser.add_argument('-p', '--port', default=5999, type=int, nargs=1,
        help='The port to connect pull from on localhost.')

args = parser.parse_args()

def create_image(width, height, data, channels=3):
    img = cv.CreateImageHeader((width, height), cv.IPL_DEPTH_8U, channels)
    buf = StringIO(data)
    cv.SetData(img, buf.read())
    return img

n = 0
@pipeline(in_ports=args.port)
def dumper(rawstream):
    global n
    img = create_image(*args.size, data=rawstream)
    cv.SaveImage(args.filepath % n, img)
    n += 1

print "Filepath: ", args.filepath
print args.size
dumper.run()
