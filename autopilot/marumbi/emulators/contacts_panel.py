# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.


class ContactsPanel(object):
    """An emulator class that makes it easy to interact with the contacts panel."""

    def __init__(self, app):
        self.app = app

    def get_contacts_searchbox(self):
        """Returns the main searchbox attached to the contacts panel."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        qdl_qdi = qdi.select_many("QDeclarativeLoader")[0].select_many("QDeclarativeItem")[0]
        return qdl_qdi.select_many("SearchEntry")[0]

    def get_add_contact_list_item(self):
        """Returns the 'Add a new contact' list item."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        qdl_qdi = qdi.select_many("QDeclarativeLoader")[0].select_many("ContactsPanel")[0]
        return qdl_qdi.select_single("ListItem")
