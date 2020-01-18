#!/usr/bin/python3

# This test opens the Help menu and runs through the menu items.

from testCommon import run_app, bail

from dogtail.procedural import *

try:
    run_app()

    # Contents
    click('Help', roleName='menu')
    click('Contents', roleName='menu item')
    focus.dialog('Text Editor')
    keyCombo('<Control>w')

    # Keyboard Shortcuts
    click('Help', roleName='menu')
    click('Keyboard Shortcuts', roleName='menu item')
    keyCombo('<Alt>F4')

    # About
    click('Help', roleName='menu')
    click('About', roleName='menu item')
    focus.dialog('About xed')
    click('Close', roleName='push button')

    # Quit application
    click('File', roleName='menu')
    click('Quit', roleName='menu item')

except:
    bail()
