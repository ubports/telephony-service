# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class CommunicationPanel(object):
    """An emulator class that makes it easy to interact with the communication
    panel.

    """
    def __init__(self, app):
        self.app = app

    def get_communication_searchbox(self):
        """Returns the main searchbox attached to the communication panel."""
        return self.app.select_single("TextField", placeholderText="Search communication")

    def get_communication_searchbox_clear_button(self):
        """Returns the clear button in the main searchbox attached to the communication panel."""
        return self.get_communication_searchbox().get_children_by_type("QQuickBorderImage")[0].get_children_by_type("Button")[0]

    def get_new_message_button(self):
        """Returns 'New Message' list item."""
        return self.app.select_single("Button", objectName='compose', text='Compose')

    def get_communication_view(self):
        """Returns the CommunicationView."""
        return self.app.select_single("CommunicationView")

    def get_new_message_send_to_box(self):
        """Return the "To" input box for sending an sms."""
        return self.app.select_single("NewMessageHeader")

    def get_message_send_button(self):
        """Returns the send button."""
        return self.app.select_single("Button", text='Send')

    def get_new_message_text_box(self):
        """Returns main message box for sending an sms."""
        return self.app.select_single("TextField", objectName="newMessageText")
