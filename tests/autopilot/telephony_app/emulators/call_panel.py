# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of telephony-app.
#
# telephony-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class CallPanel(object):
    """An emulator class that makes it easy to interact with the call panel."""

    def __init__(self, app):
        self.app = app

    def get_keypad_entry(self):
        """Returns keypad's input box."""
        return self.app.select_single('KeypadEntry')
    
    def get_erase_button(self):
        """Returns the erase button of the keypad"""
        return self.app.select_single('CustomButton', objectName="eraseButton")
    
    def get_keypad_keys(self):
        """Returns list of dialpad keys."""
        return self.app.select_many('KeypadButton')

    def get_dial_button(self):
        """Returns the Dial button"""
        return self.app.select_single('AbstractButton', objectName='callButton')

    def get_contacts_list_button(self):
        """Returns the Contacts list button next to the dial button"""
        return self.app.select_single('CustomButton', objectName='contactListButton')

    def get_dialer_page(self):
        return self.app.select_single('Tab', objectName='callsTab')

    def get_contacts_page(self):
        return self.app.select_single('Tab', objectName='contactsTab')
