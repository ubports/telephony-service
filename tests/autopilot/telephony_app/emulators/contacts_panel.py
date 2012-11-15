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

    def get_contacts_searchbox(self):
        """Returns the main searchbox attached to the contacts panel."""
        return self.app.select_single("TextField", placeholderText="Search contacts")

    def get_contacts_searchbox_icon(self, searchbox):
        """Returns the icon of the searchbox attached to the contacts panel."""
        # FIXME: this is a bit hackish, but it was the way I (boiko) managed to
        # select the correct button
        return searchbox.get_children()[0].get_children_by_type("Button")[0]

    def get_contacts_panel(self):
        """Returns the ContactsPanel."""
        return self.app.select_single("ContactsPanel")

    def get_add_contact_list_item(self):
        """Returns the 'Add a new contact' list item."""
        return self.app.select_single("LabelVisual", text='Add a new contact')

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

    def get_contact_edit_save_button(self):
        """Returns the contact edit/save button."""
        return self.app.select_single("Button", objectName="editSaveButton")
