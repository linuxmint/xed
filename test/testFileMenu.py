#!/usr/bin/python3

# Test that the File menu and menu items work correctly.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='LoremIpsum.txt')

    # Save a Copy
    click('File', roleName='menu')
    click('Save As...', roleName='menu item')
    click('Cancel', roleName='push button')

    # Print Preview
    focus.application('xed')
    click('File', roleName='menu')
    click('Print Preview', roleName='menu item')
    click('Close preview', roleName='push button')

    # Print
    focus.application('xed')
    click('File', roleName='menu')
    click('Print...', roleName='menu item')
    focus.dialog('Print')
    click('Page Setup', roleName='page tab')
    click('Text Editor', roleName='page tab')
    click('Cancel', roleName='push button')

    # Quit application
    focus.application('xed')
    click('File', roleName='menu')
    click('Quit', roleName='menu item')

except:
    bail()
