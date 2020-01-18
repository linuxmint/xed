#!/usr/bin/python3

import os
import sys
import signal

os.environ['LANG'] = 'C'

from dogtail.config import config
config.logDebugToStdOut = True
config.logDebugToFile = False

import dogtail.procedural as dt

def run_app(file=None):
    global pid

    if file is not None:
        arguments = os.path.join(os.path.dirname(__file__), file)
    else:
        arguments = ''
    pid = dt.run(sys.argv[1], arguments=arguments, appName='xed')

def bail():
    os.kill(pid, signal.SIGTERM)
    sys.exit(1)
