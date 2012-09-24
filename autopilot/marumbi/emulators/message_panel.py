# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from __future__ import absolute_import

from marumbi.tests import MarumbiTestCase

class MessagesPanel(MarumbiTestCase):
    """An emulator class that makes it easy to interact with the messages panel."""

    def get_messages_searchbox(self):
        """Returns the main searchbox attached to the messages panel."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        qdl_qdi = qdi.select_many("QDeclarativeLoader")[0].select_many("QDeclarativeItem")[0]
        return qdl_qdi.select_many("SearchEntry")[0]

    def get_new_message_list_item(self):
        """Returns 'New Message' list item."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        qdl_qdi = qdi.select_many("QDeclarativeLoader")[0].select_many("QDeclarativeItem")[0]
        return qdl_qdi.select_single("ListItem")

    def click_new_message_list_item(self):
        """Moves the mouse over 'New Message' item in the call panel and activates it."""
        new_message_item = self.get_new_message_list_item()

        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

    def get_messages_view(self):
        """Returns the MessagesView."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[1]
        return qdi.select_many("QDeclarativeLoader")[0].select_single("MessagesView")

    def get_new_message_send_to_box(self):
        """Return the "To" input box for sending an sms."""
        messages_view = self.get_messages_view()
        return messages_view.select_many("QDeclarativeLoader")[0].select_single("NewMessageHeader")

    def get_message_send_button(self):
        """Returns the send button."""
        messages_view = self.get_messages_view()
        mf_qdi =  messages_view.select_single("MessagesFooter").select_many("QDeclarativeItem")[0]
        return mf_qdi.select_many("Button")[0]

    def get_new_message_text_box(self):
        """Returns main message box for sending an sms."""
        messages_view = self.get_messages_view()
        mf_qdi =  messages_view.select_single("MessagesFooter").select_many("QDeclarativeItem")[0]
        return mf_qdi.select_many("SearchEntry")[1]

