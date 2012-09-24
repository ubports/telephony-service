# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from __future__ import absolute_import

from marumbi.tests import MarumbiTestCase


class CallPanel(MarumbiTestCase):
    """An emulator class that makes it easy to interact with the call panel."""

    def get_searchbox(self):
        """Returns the main searchbox attached to the call panel."""
        return self.get_call_panel().select_single("ContactsSearchCombo")

    def get_call_log_view_tabs(self):
        """Returns tabs inside call log item."""
        call_log_view = self.get_call_log_view()
        return call_log_view.select_many("Tabs")[0].select_many("QDeclarativeRow")[0].select_many("TabButton")

    def get_call_panel(self):
        """Returns the CallPanel."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        return qdi.select_many("QDeclarativeLoader")[0].select_single("CallPanel")

    def get_keypad_view(self):
        """Returns the KeypadView."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[1]
        return qdi.select_many("QDeclarativeLoader")[0].select_single("KeypadView")

    def get_call_log_view(self):
        """Returns the CallLog view."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[1]
        return qdi.select_many("QDeclarativeLoader")[0].select_single("CallLog")

    def get_keypad_list_item(self):
        """Returns "Keypad" list item."""
        call_panel = self.get_call_panel()
        return call_panel.select_many("QDeclarativeColumn")[0].select_many("ListItem")[0]

    def get_voicemail_list_item(self):
        """Returns "Voicemail" list item."""
        call_panel = self.get_call_panel()
        return call_panel.select_many("QDeclarativeColumn")[0].select_many("ListItem")[1]

    def get_call_log_list_item(self):
        """Returns 'Call Log' list item."""
        call_panel = self.get_call_panel()
        return call_panel.select_many("QDeclarativeColumn")[0].select_many("ListItem")[2]

    def get_keypad_entry(self):
        """Returns keypad's input box."""
        keypad_view = self.get_keypad_view()
        return keypad_view.select_many("QDeclarativeItem")[0].select_single("KeypadEntry")

    def get_keypad_keys(self):
        """Returns list of dialpad keys."""
        keypad_view = self.get_keypad_view()
        qdi_ke = keypad_view.select_many("QDeclarativeItem")[0].select_single("Keypad")
        return qdi_ke.select_single("QDeclarativeGrid").select_many("KeypadButton")

    def click_inside_searchbox(self):
        """Click inside the searchbox of the call panel."""
        csc = self.get_call_panel().select_single("ContactsSearchCombo")

        self.mouse.move_to_object(csc)
        self.mouse.click()

    def click_keypad_list_item(self):
        """Moves the mouse over 'Keypad' item in the call panel and activates it."""
        keypad_item = self.get_keypad_list_item()

        self.mouse.move_to_object(keypad_item)
        self.mouse.click()

    def click_call_log_list_item(self):
        """Moves the mouse over 'Call Log' item in the call panel and activates it."""
        call_log_item = self.get_call_log_list_item()

        self.mouse.move_to_object(call_log_item)
        self.mouse.click()


        