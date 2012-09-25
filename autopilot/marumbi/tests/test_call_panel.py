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


class TestCallPanel(MarumbiTestCase):
    """Tests for the Call panel."""

    def click_inside_searchbox(self):
        call_panel = self.call_panel.get_call_panel()
        csc = call_panel.select_single("ContactsSearchCombo")

        self.mouse.move_to_object(csc)
        self.mouse.click()

    def click_keypad_list_item(self):
        keypad_item = self.call_panel.get_keypad_list_item()

        self.mouse.move_to_object(keypad_item)
        self.mouse.click()

    def click_call_log_list_item(self):
        call_log_item = self.call_panel.get_call_log_list_item()

        self.mouse.move_to_object(call_log_item)
        self.mouse.click()

    def test_main_tab_focus(self):
        """Ensures call panel tab is pre-selected when the app is started."""
        call_tab = self.get_main_view_tabs()[0]

        self.assertThat(call_tab.selected, Eventually(Equals(True)))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        self.click_inside_searchbox()
        searchbox = self.call_panel.get_searchbox()

        self.assertThat(searchbox.activeFocus, Eventually(Equals(True)))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        self.click_inside_searchbox()
        searchbox = self.call_panel.get_searchbox()

        self.keyboard.type("test")

        self.assertThat(searchbox.searchQuery, Eventually(Equals("test")))

    def test_keypad_view_active(self):
        """Click the 'Keypad' list item must show the keypad."""
        self.click_keypad_list_item()
        keypad_view = self.call_panel.get_keypad_view()

        self.assertThat(keypad_view.activeFocus, Eventually(Equals(True)))

    def test_keypad_click(self):
        """Clicking on the dialpad keys must show the numbers associated to
        the keys.

        """
        self.click_keypad_list_item()
        keypad_keys = self.call_panel.get_keypad_keys()
        keypad_entry = self.call_panel.get_keypad_entry()

        for keys in keypad_keys:
            self.mouse.move_to_object(keys)
            self.mouse.click()

        self.assertThat(keypad_entry.value, Eventually(Equals("123456789*0#")))

    def test_keypad_entry_delete(self):
        """Clicking the back button on the keypad must remove a numbers."""
        self.click_keypad_list_item()
        entry = self.call_panel.get_keypad_entry()
        delete_button = entry.select_single("AbstractButton")

        self.keyboard.type("911")

        self.mouse.move_to_object(delete_button)

        self.mouse.click()
        self.assertThat(entry.value, Eventually(Equals("91")))
        self.mouse.click()
        self.assertThat(entry.value, Eventually(Equals("9")))
        self.mouse.click()
        self.assertThat(entry.value, Eventually(Equals("")))

    def test_keypad_entry_typing(self):
        """Ensures that typing with the keyboard also works in keypad."""
        self.click_keypad_list_item()
        entry = self.call_panel.get_keypad_entry()

        self.keyboard.type("911")

        self.assertThat(entry.value, Eventually(Equals("911")))

    def test_call_log_first_tab_focus(self):
        """Ensures that 'All' tab is pre-selected when 'Call Log' view
        is shown.

        """
        self.click_call_log_list_item()
        first_call_log_tab = self.call_panel.get_call_log_view_tabs()[3]

        self.assertThat(first_call_log_tab.selected, Eventually(Equals(True)))

    def test_call_log_second_tab_focus(self):
        """Clicking on the 'Missed' tab must switch to it."""
        self.click_call_log_list_item()
        second_call_log_tab = self.call_panel.get_call_log_view_tabs()[4]

        self.mouse.move_to_object(second_call_log_tab)
        self.mouse.click()

        self.assertThat(second_call_log_tab.selected, Eventually(Equals(True)))
