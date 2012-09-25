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


class TestMessagesPanel(MarumbiTestCase):
    """Tests for the messages panel."""

    def setUp(self):
        super(TestMessagesPanel, self).setUp()
        messages_tab = self.get_main_view_tabs()[1]
        self.mouse.move_to_object(messages_tab)
        self.mouse.click()

    def click_new_message_list_item(self):
        new_message_item = self.messages_panel.get_new_message_list_item()

        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

    def test_main_tab_focus(self):
        """Clicking on the 'messages' tab must give it the focus."""
        messages_tab = self.messages_panel.get_main_view_tabs()[1]

        self.assertThat(messages_tab.selected, Eventually(Equals(True)))

    def test_searchbox_focus(self):
        """Clicking inside the searbox must give it the focus."""
        searchbox = self.messages_panel.get_messages_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.assertThat(searchbox.activeFocus, Eventually(Equals(True)))

    def test_searchbox_entry(self):
        """Ensures that typing inside the main searchbox works."""
        searchbox = self.messages_panel.get_messages_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(searchbox.searchQuery, Eventually(Equals("test")))

    def test_searchbox_clear_button(self):
        """clicking the cross icon must clear the searchbox."""
        searchbox = self.messages_panel.get_messages_searchbox()
        self.mouse.move_to_object(searchbox)
        self.mouse.click()

        self.keyboard.type("test")

        (x, y, w, h) = searchbox.globalRect

        # I am not able to find anything about the clear icon
        # so moving the mouse exactly to that icon with some calculations
        tx = x + (w / 12)
        ty = y + (h / 2)

        self.mouse.move(tx, ty)
        self.mouse.click()

        self.assertThat(searchbox.searchQuery, Eventually(Equals("")))

    def test_new_message_list_item_select(self):
        """Clicking on the 'New Message' list item must highlight it."""
        new_message_item = self.messages_panel.get_new_message_list_item()

        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

        self.assertThat(new_message_item.selected, Eventually(Equals(True)))

    def test_message_view_visible(self):
        """Clicking on the 'New Message' list item must show the
        message view.

        """
        new_message_item = self.messages_panel.get_new_message_list_item()
        self.mouse.move_to_object(new_message_item)
        self.mouse.click()

        message_view = self.get_messages_view()

        self.assertThat(message_view.visible, Eventually(Equals(True)))

    def test_message_send_to_focus(self):
        """Clicking the 'New Message' list item must give focus to the
        'sendto' box.

        """
        self.click_new_message_list_item()
        sendto_box = self.messages_panel.get_new_message_send_to_box()

        self.assertThat(sendto_box.activeFocus, Eventually(Equals(True)))

    def test_message_send_to_entry(self):
        """Ensures that number can be typed into the 'sendto' box."""
        self.click_new_message_list_item()
        sendto_box = self.messages_panel.get_new_message_send_to_box()

        self.keyboard.type("911")

        self.assertThat(sendto_box.text, Eventually(Equals("911")))

    def test_send_button_active(self):
        """Typing a number into the 'sendto' box must enable the Send
        button.

        """
        self.click_new_message_list_item()
        send_button = self.messages_panel.get_message_send_button()

        self.keyboard.type("911")

        self.assertThat(send_button.state, Eventually(Equals("idle")))

    def test_send_button_disable_on_clear(self):
        """Removing the number from the 'sendto' box must disable the
        Send button.

        """
        self.click_new_message_list_item()
        send_button = self.messages_panel.get_message_send_button()

        self.keyboard.type("911")
        self.keyboard.press_and_release("Ctrl+a")
        self.keyboard.press_and_release("Delete")

        self.assertThat(send_button.state, Eventually(Equals("disabled")))

    def test_new_message_box_focus(self):
        """Clicking inside the main message box must give it the focus."""
        self.click_new_message_list_item()
        message_box = self.messages_panel.get_new_message_text_box()

        self.mouse.move_to_object(message_box)
        self.mouse.click()

        self.assertThat(message_box.activeFocus, Eventually(Equals(True)))

    def test_new_message_box_entry(self):
        """Ensures that typing inside the main message box works."""
        self.click_new_message_list_item()
        message_box = self.messages_panel.get_new_message_text_box()

        self.mouse.move_to_object(message_box)
        self.mouse.click()

        self.keyboard.type("test")

        self.assertThat(message_box.searchQuery, Eventually(Equals("test")))
