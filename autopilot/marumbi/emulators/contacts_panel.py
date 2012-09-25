# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class ContactsPanel(object):
    """An emulator class that makes it easy to interact with the contacts
    panel.

    """
    def __init__(self, app):
        self.app = app

    def get_parent_q_declarative_item(self):
        qdv = self.app.select_single("QDeclarativeView")
        qgs = qdv.select_single("QGraphicsScene")
        return qgs.select_many("QDeclarativeItem")[0]

    def get_contacts_searchbox(self):
        """Returns the main searchbox attached to the contacts panel."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[0]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        qdi_2 = qdl.select_many("QDeclarativeItem")[0]
        return qdi_2.select_many("SearchEntry")[0]

    def get_add_contact_list_item(self):
        """Returns the 'Add a new contact' list item."""
        parent_qdi = self.get_parent_q_declarative_item()
        qdi = parent_qdi.select_many("QDeclarativeItem")[0]
        qdl = qdi.select_many("QDeclarativeLoader")[0]
        cp = qdl.select_many("ContactsPanel")[0]
        return cp.select_many("ListItem")[0]
