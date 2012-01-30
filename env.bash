#!/bin/bash

if [[ "$BASH_SOURCE" == "$0" ]]
then
    echo usage: \$ source $0
    exit 1
fi

LAARK_PATH="$( cd "$( dirname "$0" )" && pwd )"
export PYTHONPATH=$PYTHONPATH:$LAARK_PATH/python
export PATH=$LAARK_PATH/bin:$PATH
