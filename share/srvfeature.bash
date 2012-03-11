#!/bin/bash

port=${1:-8000}
inbound=${2:-5999}

cleanup() {
    kill -s 9 $httpid
    rm -rf *.png
    exit 0
}

trap cleanup SIGHUP SIGINT SIGTERM

python -m SimpleHTTPServer $port 2>&1 > /dev/null &
httpid=$!

echo $httpid

python $LAARK/python/laark/pipeline/goodfeatures.py -i $inbound -n 10 2>&1 > /dev/null
