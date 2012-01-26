#!/bin/bash

LAARK_PATH="$( cd "$( dirname "$0" )" && pwd )"
export PYTHONPATH=$PYTHONPATH:$LAARK_PATH/python
export PATH=$LAARK_PATH/bin:$PATH
