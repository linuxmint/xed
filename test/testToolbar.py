#!/usr/bin/python3

# Test that the Toolbar works correctly.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app(file='LoremIpsum.txt')

    # NEW 
    click('New', roleName='push button')
    focus.widget('LoremIpsum.txt', roleName='page tab')
    focus.widget.node.select()

    # OPEN
    click('Open', roleName='push button')
    click('Cancel', roleName='push button')
 
      
    focus.application('xed')
    click('Edit', roleName='menu')
    click('Select All', roleName='menu item')
    focus.application('xed')
    type('testing')

    # UNDO
    click('Undo', roleName='push button')
    # REDO
    click('Redo', roleName='push button')

    # THE ISSUE OF NEEDING TO PRESS UNDO TWICE AS BEEN DOCUMENTED AS A 
    # FEATURE REQUEST (#344)
    # ONCE THAT FR HAS BEEN IMPLEMENTED, ONE OF THESE LINES CAN BE REMOVED
    click('Undo', roleName='push button')
    click('Undo', roleName='push button')

    # CUT
    keyCombo('<Ctrl>a')
    click('Cut', roleName='push button')

    # PASTE
    click('Paste', roleName='push button')

    # COPY
    keyCombo('<Ctrl>a')
    click('Copy', roleName='push button')
    click('New', roleName='push button')
    click('Paste', roleName='push button')

    # SEARCH
    click('Find', roleName='push button')
    type('search test')

    # REPLACE
    click('Find and Replace', roleName='push button')
    type('replace test')

    # QUIT APPLICATION
    focus.application('xed')
    click('File', roleName='menu')
    click('Quit', roleName='menu item')
    click('Close without Saving', roleName='push button')

except:
    bail()
