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
        return self.get_call_panel().select_single("ContactsSearchCombo")

    def get_call_log_view_tabs(self):
        """Returns tabs inside call log item."""
        call_log_view = self.get_call_log_view()
        tbs = call_log_view.select_many("Tabs")[0]
        qdr = tbs.select_many("QDeclarativeRow")[0]
        return qdr.select_many("TabButton")

    def get_parent_q_declarative_item(self):
        qdv = self.app.select_single("QDeclarativeView")
        qgs = qdv.select_single("QGraphicsScene")
        return qgs.select_many("QDeclarativeItem")[0]

    def get_call_panel(self):
        """Returns the CallPanel."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[0]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        return qdl.select_single("CallPanel")

    def get_keypad_view(self):
        """Returns the KeypadView."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[1]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        return qdl.select_single("KeypadView")

    def get_call_log_view(self):
        """Returns the CallLog view."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[1]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        return qdl.select_single("CallLog")

    def get_keypad_list_item(self):
        """Returns "Keypad" list item."""
        call_panel = self.get_call_panel()
        qdc = call_panel.select_many("QDeclarativeColumn")[0]
        return qdc.select_many("ListItem")[0]

    def get_voicemail_list_item(self):
        """Returns "Voicemail" list item."""
        call_panel = self.get_call_panel()
        qdc = call_panel.select_many("QDeclarativeColumn")[0]
        return qdc.select_many("ListItem")[1]

    def get_call_log_list_item(self):
        """Returns 'Call Log' list item."""
        call_panel = self.get_call_panel()
        qdc = call_panel.select_many("QDeclarativeColumn")[0]
        return qdc.select_many("ListItem")[2]

    def get_keypad_entry(self):
        """Returns keypad's input box."""
        keypad_view = self.get_keypad_view()
        qdi = keypad_view.select_many("QDeclarativeItem")[0]
        return qdi.select_single("KeypadEntry")

    def get_keypad_keys(self):
        """Returns list of dialpad keys."""
        keypad_view = self.get_keypad_view()
        qdi = keypad_view.select_many("QDeclarativeItem")[0]
        kp_qdg = qdi.select_single("Keypad").select_single("QDeclarativeGrid")
        return kp_qdg.select_many("KeypadButton")
