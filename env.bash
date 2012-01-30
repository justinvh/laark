#!/bin/bash

if [[ "$BASH_SOURCE" == "$0" ]]
then
    echo usage: \$ source $0
    exit 1
fi

export LAARK="$( cd "$( dirname "$0" )" && pwd )"
export PYTHONPATH=$PYTHONPATH:$LAARK/python
export PATH=$LAARK/bin:$PATH
