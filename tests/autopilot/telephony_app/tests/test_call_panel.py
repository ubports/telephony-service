# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of telephony-app.
#
# telephony-app is free software: you can redistribute it and/or modify it
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

        for keys in keypad_keys:
            self.pointing_device.move_to_object(keys)
            self.pointing_device.click()
        
        self.assertThat(keypad_entry.value, Eventually(Equals("123456789*0#")))

    def test_call_button_disabling(self):
        """The call button needs to be disabled when there is no number in the input"""
        keypad_entry = self.call_panel.get_keypad_entry()
        keypad_keys = self.call_panel.get_keypad_keys()
        dial_button = self.call_panel.get_dial_button()
        self.assertThat(keypad_entry.value, Eventually(Equals("")))
        self.assertThat(dial_button.enabled, Eventually(Equals(False)))

        self.pointing_device.move_to_object(keypad_keys[0])
        self.pointing_device.click()
        self.pointing_device.move_to_object(keypad_keys[1])
        self.pointing_device.click()
        self.pointing_device.move_to_object(keypad_keys[2])
        self.pointing_device.click()

        self.assertThat(keypad_entry.value, Eventually(Equals("123")))
        self.assertThat(dial_button.enabled, Eventually(Equals(True)))

        erase_button = self.call_panel.get_erase_button()
        self.pointing_device.move_to_object(erase_button)
        self.pointing_device.click()
        self.pointing_device.click()
        self.pointing_device.click()
        self.assertThat(dial_button.enabled, Eventually(Equals(False)))

    def test_call(self):
        """Dialing a number works"""
        keypad_entry = self.call_panel.get_keypad_entry()
        keypad_keys = self.call_panel.get_keypad_keys()

        self.pointing_device.move_to_object(keypad_keys[0])
        self.pointing_device.click()
        self.pointing_device.move_to_object(keypad_keys[1])
        self.pointing_device.click()
        self.pointing_device.move_to_object(keypad_keys[2])
        self.pointing_device.click()
        self.pointing_device.move_to_object(keypad_keys[3])
        self.pointing_device.click()

        self.assertThat(keypad_entry.value, Eventually(Equals("1234")))
        
        dial_button = self.call_panel.get_dial_button()
        self.pointing_device.move_to_object(dial_button)
        self.pointing_device.click()
        
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
        self.pointing_device.move_to_object(contacts_list_button)
        self.pointing_device.click()
        
        self.assertThat(dialer_page.isCurrent, Eventually(Equals(False)))
        self.assertThat(contacts_page.isCurrent, Eventually(Equals(True)))
