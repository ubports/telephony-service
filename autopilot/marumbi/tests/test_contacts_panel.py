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

from marumbi.tests import MarumbiTestCase
from marumbi.emulators.contacts_panel import ContactsPanel


class TestContactsPanel(ContactsPanel, MarumbiTestCase):

    def setUp(self):
        super(TestContactsPanel, self).setUp()
        contacts_tab = self.get_main_view_tabs()[2]
        self.mouse.move_to_object(contacts_tab)
        self.mouse.click()

    def test_main_tab_focus(self):
        """Clicking on the 'contacts' tab must give it the focus."""
        contacts_tab = self.get_main_view_tabs()[2]

        self.assertThat(contacts_tab.selected, Equals(True))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        searchbox = self.get_contacts_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.assertThat(searchbox.activeFocus, Equals(True))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        searchbox = self.get_contacts_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(searchbox.searchQuery, Eventually(Equals("test")))

    def test_searchbox_clear_button(self):
        """clicking the cross icon must clear the searchbox."""
        searchbox = self.get_contacts_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        (x, y, w, h) = searchbox.globalRect

        # I am not able to find anything about the 'clear' icon
        # so moving the mouse exactly to that icon with some calculations
        tx = x + (w / 12)
        ty = y + (h / 2)

        self.mouse.move(tx, ty)
        self.mouse.click()

        self.assertThat(searchbox.searchQuery, Eventually(Equals("")))

    def test_add_new_contact_list_item_select(self):
        """Clicking on the 'Add a new contact' list item must highlight it."""
        new_contact_item = self.get_add_contact_list_item()

        self.mouse.move_to_object(new_contact_item)
        self.mouse.click()

        self.assertThat(new_contact_item.selected, Eventually(Equals(True)))
