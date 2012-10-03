# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class CallPanel(object):
    """An emulator class that makes it easy to interact with the call panel."""

    def __init__(self, app):
        self.app = app

    def get_searchbox(self):
        """Returns the main searchbox attached to the call panel."""
        return self.app.select_single("ContactsSearchCombo")

    def get_call_log_view_tabs(self):
        """Returns tabs inside call log item."""
        return self.app.select_many('TabButton')

    def get_call_panel(self):
        """Returns the CallPanel."""
        return self.app.select_single('CallPanel')

    def get_keypad_view(self):
        """Returns the KeypadView."""
        return self.app.select_single('KeypadView')

    def get_call_log_view(self):
        """Returns the CallLog view."""
        return self.app.select_single('CallLog')

    def get_keypad_list_item(self):
        """Returns "Keypad" list item."""
        return self.app.select_single("ListItem", text='Keypad')

    def get_voicemail_list_item(self):
        """Returns "Voicemail" list item."""
        return self.app.select_single("ListItem", text='Voicemail')

    def get_call_log_list_item(self):
        """Returns 'Call Log' list item."""
        return self.app.select_single("ListItem", text='Call Log')

    def get_keypad_entry(self):
        """Returns keypad's input box."""
        return self.app.select_single('KeypadEntry')

    def get_keypad_keys(self):
        """Returns list of dialpad keys."""
        return self.app.select_many('KeypadButton')
