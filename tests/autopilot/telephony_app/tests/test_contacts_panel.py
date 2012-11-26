# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Tests for the Telephony App"""

from __future__ import absolute_import

from testtools.matchers import Equals
from autopilot.matchers import Eventually

from telephony_app.tests import TelephonyAppTestCase

import time

class TestContactsPanel(TelephonyAppTestCase):
    """Tests for the contacts panel."""

    def setUp(self):
        super(TestContactsPanel, self).setUp()
        contacts_tab = self.get_main_view_tabs()[2]
        self.mouse.move_to_object(contacts_tab)
        self.mouse.click()

    def click_add_contact_list_item(self):
        new_contact_item = self.contacts_panel.get_add_contact_list_item()

        self.mouse.move_to_object(new_contact_item)
        self.mouse.click()

        return new_contact_item

    def test_main_tab_focus(self):
        """Clicking on the 'contacts' tab must give it the focus."""
        contacts_tab = self.get_main_view_tabs()[2]

        self.assertThat(contacts_tab.selected, Eventually(Equals(True)))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.assertThat(searchbox.activeFocus, Eventually(Equals(True)))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(searchbox.text, Eventually(Equals("test")))

    def test_searchbox_clear_button(self):
        """Clicking the cross icon must clear the searchbox."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        clearicon = self.contacts_panel.get_contacts_searchbox_icon(searchbox)
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        self.mouse.move_to_object(clearicon)
        self.mouse.click()

        self.assertThat(searchbox.text, Eventually(Equals("")))

    def test_add_new_contact_page(self):
        """Clicking on the 'Add a new contact' list item must load the contact details page."""
        self.click_add_contact_list_item()
        contact_details = self.contacts_panel.get_contact_details()

        self.assertThat(contact_details.visible, Eventually(Equals(True)))

    def test_add_new_contact_first_name_focused(self):
        """When adding a new contact, the first name input field needs to be focused by default."""
        self.click_add_contact_list_item()
        first_name = self.contacts_panel.get_contact_first_name_input()

        self.assertThat(first_name.activeFocus, Eventually(Equals(True)))

    def test_add_new_contact_with_name(self):
        """Test adding a contact with first and last names set."""
        self.click_add_contact_list_item()

        first_name = self.contacts_panel.get_contact_first_name_input()
        self.keyboard.type("FirstName")

        last_name = self.contacts_panel.get_contact_last_name_input()
        self.mouse.move_to_object(last_name)
        self.mouse.click()
        self.keyboard.type("LastName")

        save_button = self.contacts_panel.get_contact_save_button()
        self.mouse.move_to_object(save_button)
        self.mouse.click()

        contacts_list = self.contacts_panel.get_contact_list()
        self.assertThat(contacts_list.count, Eventually(Equals(1)))

