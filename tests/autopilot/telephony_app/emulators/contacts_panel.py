# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of telephony-app.
#
# telephony-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

from telephony_app.emulators.utils import Utils

class ContactsPanel(Utils):
    """An emulator class that makes it easy to interact with the contacts
    panel.

    """
    def __init__(self, app):
        Utils.__init__(self, app)

    def get_contacts_searchbox(self):
        """Returns the main searchbox attached to the contacts panel."""
        return self.app.select_single("TextField", objectName="contactsSearchBox")

    def get_contacts_searchbox_icon(self, searchbox):
        """Returns the icon of the searchbox attached to the contacts panel."""
        # FIXME: this is a bit hackish, but it was the way I (boiko) managed to
        # select the correct button
        return self.get_contacts_searchbox().get_children_by_type("AbstractButton")[0]

    def get_contacts_panel(self):
        """Returns the ContactsPanel."""
        return self.app.select_single("ContactsPanel")

    def get_add_contact_button(self):
        """Returns the 'Add a new contact' list item."""
        return self.get_tool_button(0)

    def get_contact_list(self):
        """Returns the contact list item."""
        return self.app.select_single("QQuickListView", objectName="contactsList")

    def get_contact_details(self):
        """Returns the ContactDetails page."""
        return self.app.select_single("ContactDetails")

    def get_contact_first_name_input(self):
        """Returns the contact first name input item."""
        return self.app.select_single("AddressEditorField", objectName="firstName")

    def get_contact_last_name_input(self):
        """Returns the contact last name input item."""
        return self.app.select_single("AddressEditorField", objectName="lastName")

    def get_contact_edit_button(self):
        """Returns the contact edit button."""
        return self.get_tool_button(0)

    def get_contact_save_button(self):
        """Returns the contact save button."""
        return self.get_tool_button(3)
