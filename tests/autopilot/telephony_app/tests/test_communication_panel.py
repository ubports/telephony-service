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


class TestCommunicationPanel(TelephonyAppTestCase):
    """Tests for the communication panel."""

    def setUp(self):
        super(TestCommunicationPanel, self).setUp()
        communication_page = self.communication_panel.get_communication_page()
        self.move_to_next_tab()
        self.assertThat(communication_page.isCurrent, Eventually(Equals(True)))

    def click_new_message_button(self):
        new_message_item = self.communication_panel.get_new_message_button()

        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

    def click_sendto_box(self):
        sendto_box = self.communication_panel.get_new_message_send_to_box()
        self.mouse.move_to_object(sendto_box)
        self.mouse.click()
        self.assertThat(sendto_box.activeFocus, Eventually(Equals(True)))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        searchbox = self.communication_panel.get_communication_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.assertThat(searchbox.activeFocus, Eventually(Equals(True)))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        searchbox = self.communication_panel.get_communication_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(searchbox.text, Eventually(Equals("test")))

    def test_searchbox_clear_button(self):
        """Clicking the cross icon must clear the searchbox."""
        searchbox = self.communication_panel.get_communication_searchbox()
        clear_button = self.communication_panel.get_communication_searchbox_clear_button()

        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")
        self.assertThat(searchbox.text, Eventually(Equals("test")))

        self.mouse.move_to_object(clear_button)
        self.mouse.click()

        self.assertThat(searchbox.text, Eventually(Equals("")))

    def test_communication_view_visible(self):
        """Clicking on the 'New Message' list item must show the message view.
        """
        self.click_new_message_button()
        communication_view = self.communication_panel.get_communication_view()

        self.assertThat(communication_view.visible, Eventually(Equals(True)))

    def test_message_send_to_focus(self):
        """Clicking the 'New Message' list item must give focus to the
        'sendto' box.

        """
        self.click_new_message_button()
        sendto_box = self.communication_panel.get_new_message_send_to_box()

        # FIXME: we should have the field focused by default, but right now we
        # need to explicitly give it focus
        self.click_sendto_box()

        self.assertThat(sendto_box.activeFocus, Eventually(Equals(True)))

    def test_message_send_to_entry(self):
        """Ensures that number can be typed into the 'sendto' box."""
        self.click_new_message_button()
        sendto_box = self.communication_panel.get_new_message_send_to_box()

        # FIXME: we should have the field focused by default, but right now we
        # need to explicitly give it focus
        self.click_sendto_box()
        self.keyboard.type("911")

        self.assertThat(sendto_box.text, Eventually(Equals("911")))

    def test_send_button_active(self):
        """Typing a number into the 'sendto' box must enable the Send button.
        """
        self.click_new_message_button()
        send_button = self.communication_panel.get_message_send_button()

        # FIXME: we should have the field focused by default, but right now we
        # need to explicitly give it focus
        self.click_sendto_box()
        self.keyboard.type("911")

        self.assertThat(send_button.enabled, Eventually(Equals(True)))

    def test_send_button_disable_on_clear(self):
        """Removing the number from the 'sendto' box must disable the
        Send button.

        """
        self.click_new_message_button()
        send_button = self.communication_panel.get_message_send_button()

        # FIXME: we should have the field focused by default, but right now we
        # need to explicitly give it focus
        self.click_sendto_box()
        self.keyboard.type("911")
        self.keyboard.press_and_release("Ctrl+a")
        self.keyboard.press_and_release("Delete")

        self.assertThat(send_button.enabled, Eventually(Equals(False)))

    def test_new_message_box_focus(self):
        """Clicking inside the main message box must give it the focus."""
        self.click_new_message_button()
        message_box = self.communication_panel.get_new_message_text_box()

        self.mouse.move_to_object(message_box)
        self.mouse.click()

        self.assertThat(message_box.activeFocus, Eventually(Equals(True)))

    def test_new_message_box_entry(self):
        """Ensures that typing inside the main message box works."""
        self.click_new_message_button()
        message_box = self.communication_panel.get_new_message_text_box()

        self.mouse.move_to_object(message_box)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(message_box.text, Eventually(Equals("test")))
