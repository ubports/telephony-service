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


class TestCallPanel(TelephonyAppTestCase):
    """Tests for the Call panel."""

    def setUp(self):
        super(TestCallPanel, self).setUp()
        dialer_page = self.call_panel.get_dialer_page()
        self.assertThat(dialer_page.isCurrent, Eventually(Equals(True)))

    def test_keypad(self):
        """The Keypad works, either using a hardware keyboard or the in-app buttons"""
        keypad_entry = self.call_panel.get_keypad_entry()
        keypad_keys = self.call_panel.get_keypad_keys()

        self.mouse.move_to_object(keypad_entry)
        self.mouse.click()
        
        self.keyboard.press_and_release("1")
        self.keyboard.press_and_release("2")
        self.keyboard.press_and_release("3")
        
        self.assertThat(keypad_entry.value, Eventually(Equals("123")))
        
        self.keyboard.press_and_release("BackSpace")
        self.keyboard.press_and_release("BackSpace")
        self.keyboard.press_and_release("BackSpace")
        self.assertThat(keypad_entry.value, Eventually(Equals("")))
        
        for keys in keypad_keys:
            self.mouse.move_to_object(keys)
            self.mouse.click()
        
        self.assertThat(keypad_entry.value, Eventually(Equals("123456789*0#")))

    def test_call_button_disabling(self):
        """The call button needs to be disabled when there is no number in the input"""
        keypad_entry = self.call_panel.get_keypad_entry()
        dial_button = self.call_panel.get_dial_button()
        self.assertThat(keypad_entry.value, Eventually(Equals("")))
        self.assertThat(dial_button.enabled, Eventually(Equals(False)))

        self.mouse.move_to_object(keypad_entry)
        self.mouse.click()
        self.keyboard.press_and_release("1");
        self.keyboard.press_and_release("2");
        self.keyboard.press_and_release("3");
        self.assertThat(keypad_entry.value, Eventually(Equals("123")))
        self.assertThat(dial_button.enabled, Eventually(Equals(True)))

        self.keyboard.press_and_release("BackSpace")
        self.keyboard.press_and_release("BackSpace")
        self.keyboard.press_and_release("BackSpace")
        self.assertThat(dial_button.enabled, Eventually(Equals(False)))

    def test_call(self):
        """Dialing a number works"""
        keypad_entry = self.call_panel.get_keypad_entry()
        self.mouse.move_to_object(keypad_entry)
        self.mouse.click()

        self.keyboard.press_and_release("1");
        self.keyboard.press_and_release("2");
        self.keyboard.press_and_release("3");
        self.keyboard.press_and_release("4");
        self.assertThat(keypad_entry.value, Eventually(Equals("1234")))
        
        dial_button = self.call_panel.get_dial_button()
        self.mouse.move_to_object(dial_button)
        self.mouse.click()
        
        # Hmpf... Nothing to evaluate right now... Lets just make sure the input
        # field cleared itself for now
        keypad_entry = self.call_panel.get_keypad_entry()
        self.assertThat(keypad_entry.value, Eventually(Equals("")))

    def test_switch_to_contacts(self):
        """Switching to the contact list using the dialers contacts button"""
        dialer_page = self.call_panel.get_dialer_page()
        contacts_page = self.call_panel.get_contacts_page()

        self.assertThat(dialer_page.isCurrent, Eventually(Equals(True)))
        self.assertThat(contacts_page.isCurrent, Eventually(Equals(False)))
        
        contacts_list_button = self.call_panel.get_contacts_list_button()
        self.mouse.move_to_object(contacts_list_button)
        self.mouse.click()
        
        self.assertThat(dialer_page.isCurrent, Eventually(Equals(False)))
        self.assertThat(contacts_page.isCurrent, Eventually(Equals(True)))
