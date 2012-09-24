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
from marumbi.emulators.call_panel import CallPanel

class TestCallPanel(CallPanel, MarumbiTestCase):
    """Tests for the Call panel."""
    
    def test_main_tab_focus(self):
        """Ensures call panel tab is pre-selected when the app is started."""
        call_tab = self.get_main_view_tabs()[0]
        
        self.assertThat(call_tab.selected, Equals(True))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        self.click_inside_searchbox()
        searchbox = self.get_searchbox()

        self.assertThat(searchbox.activeFocus, Equals(True))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        self.click_inside_searchbox()
        searchbox = self.get_searchbox()

        self.keyboard.type("test")
        
        self.assertThat(searchbox.searchQuery, Eventually(Equals("test")))

    def test_keypad_view_active(self):
        """Click the 'Keypad' list item must show the keypad."""
        self.click_keypad_list_item()
        keypad_view = self.get_keypad_view()
        
        self.assertThat(keypad_view.activeFocus, Eventually(Equals(True)))

    def test_keypad_click(self):
        """Clicking on the dialpad keys must show the numbers associated to the keys."""
        self.click_keypad_list_item()
        keypad_keys = self.get_keypad_keys()
        entry = self.get_keypad_entry()

        for keys in keypad_keys:
            self.mouse.move_to_object(keys)
            self.mouse.click()

        self.assertThat(entry.value, Eventually(Equals("123456789*0#")))

    def test_keypad_entry_delete(self):
        """Clicking the back button on the keypad must remove a numbers."""
        self.click_keypad_list_item()
        entry = self.get_keypad_entry()
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
        entry = self.get_keypad_entry()

        self.keyboard.type("911")
        
        self.assertThat(entry.value, Eventually(Equals("911")))

    def test_call_log_first_tab_focus(self):
        """Ensures that 'All' tab is pre-selected when 'Call Log' view is shown."""
        self.click_call_log_list_item()
        first_call_log_tab = self.get_call_log_view_tabs()[3]

        self.assertThat(first_call_log_tab.selected, Equals(True))

    def test_call_log_second_tab_focus(self):
        """Clicking on the 'Missed' tab must switch to it."""
        self.click_call_log_list_item()
        second_call_log_tab = self.get_call_log_view_tabs()[4]

        self.mouse.move_to_object(second_call_log_tab)
        self.mouse.click()

        self.assertThat(second_call_log_tab.selected, Equals(True))

