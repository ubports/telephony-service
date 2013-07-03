# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of phone-app.
#
# phone-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Tests for the Phone App"""

from __future__ import absolute_import

from autopilot.matchers import Eventually
from testtools.matchers import Equals, GreaterThan

from phone_app.tests import PhoneAppTestCase

import unittest

import time

class TestContactsPanel(PhoneAppTestCase):
    """Tests for the contacts panel."""

    def setUp(self):
        super(TestContactsPanel, self).setUp()
        self.switch_to_contacts_tab()

    def click_add_contact(self):
        self.reveal_toolbar()
        add_contact_button = self.contacts_panel.get_add_contact_button()

        self.pointing_device.move_to_object(add_contact_button)
        self.pointing_device.click()

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        self.pointing_device.move_to_object(searchbox)
        self.pointing_device.click()

        self.assertThat(searchbox.activeFocus, Eventually(Equals(True)))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        self.pointing_device.move_to_object(searchbox)
        self.pointing_device.click()

        self.keyboard.type("test")

        self.assertThat(searchbox.text, Eventually(Equals("test")))

    def test_searchbox_clear_button(self):
        """Clicking the cross icon must clear the searchbox."""
        searchbox = self.contacts_panel.get_contacts_searchbox()
        clearicon = self.contacts_panel.get_contacts_searchbox_icon(searchbox)
        self.pointing_device.move_to_object(searchbox)
        self.pointing_device.click()

        self.keyboard.type("test")

        self.pointing_device.move_to_object(clearicon)
        self.pointing_device.click()

        self.assertThat(searchbox.text, Eventually(Equals("")))

    def test_add_new_contact_page(self):
        """Clicking on the 'Add a new contact' list item must load the contact details page."""
        self.click_add_contact()
        contact_details = self.contacts_panel.get_contact_details()

        self.assertThat(contact_details.visible, Eventually(Equals(True)))

    def test_add_new_contact_with_name(self):
        """Test adding a contact with first and last names set."""
        self.click_add_contact()

        first_name = self.contacts_panel.get_contact_first_name_input()
        self.pointing_device.move_to_object(first_name)
        self.pointing_device.click()
        self.keyboard.type("FirstName")

        last_name = self.contacts_panel.get_contact_last_name_input()
        self.pointing_device.move_to_object(last_name)
        self.pointing_device.click()
        self.keyboard.type("LastName")

        save_button = self.contacts_panel.get_contact_save_button()
        self.pointing_device.move_to_object(save_button)
        self.pointing_device.click()

        contacts_list = self.contacts_panel.get_contact_list()
        self.assertThat(contacts_list.count, Eventually(Equals(1)))

