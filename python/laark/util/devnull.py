#!/usr/bin/env python
'''
This kills the pipeline.
'''
import argparse
from pipeline import pipeline

parser = argparse.ArgumentParser()
parser.add_argument('--in-ports', dest='in_ports', type=int, nargs='+',
                    default=[6666],
                    help='The ports that dev-null will pull from.')

args = parser.parse_args()

@pipeline(in_ports=args.in_ports)
def dev_null(_):
    pass

dev_null.run()
