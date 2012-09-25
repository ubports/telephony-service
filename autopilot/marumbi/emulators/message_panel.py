# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class MessagesPanel(object):
    """An emulator class that makes it easy to interact with the messages
    panel.

    """

    def __init__(self, app):
        self.app = app

    def get_parent_q_declarative_item(self):
        qdv = self.app.select_single("QDeclarativeView")
        qgs = qdv.select_single("QGraphicsScene")
        return qgs.select_many("QDeclarativeItem")[0]

    def get_messages_searchbox(self):
        """Returns the main searchbox attached to the messages panel."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[0]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        qdi_2 = qdl.select_many("QDeclarativeItem")[0]
        return qdi_2.select_many("SearchEntry")[0]

    def get_new_message_list_item(self):
        """Returns 'New Message' list item."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[0]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        qdi_2 = qdl.select_many("QDeclarativeItem")[0]
        return qdi_2.select_single("ListItem")

    def click_new_message_list_item(self):
        """Moves the mouse over 'New Message' item in the call panel and
        activates it.

        """
        new_message_item = self.get_new_message_list_item()

        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

    def get_messages_view(self):
        """Returns the MessagesView."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[1]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        return qdl.select_single("MessagesView")

    def get_new_message_send_to_box(self):
        """Return the "To" input box for sending an sms."""
        messages_view = self.get_messages_view()
        qdl = messages_view.select_many("QDeclarativeLoader")[0]
        return qdl.select_single("NewMessageHeader")

    def get_message_send_button(self):
        """Returns the send button."""
        messages_view = self.get_messages_view()
        mf = messages_view.select_single("MessagesFooter")
        qdi = mf.select_many("QDeclarativeItem")[0]
        return qdi.select_many("Button")[0]

    def get_new_message_text_box(self):
        """Returns main message box for sending an sms."""
        messages_view = self.get_messages_view()
        mf = messages_view.select_single("MessagesFooter")
        qdi = mf.select_many("QDeclarativeItem")[0]
        return qdi.select_many("SearchEntry")[1]
