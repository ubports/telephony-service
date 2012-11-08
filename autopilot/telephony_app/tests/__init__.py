# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Telephony App autopilot tests."""

from autopilot.introspection.qt import QtIntrospectionTestMixin
from autopilot.testcase import AutopilotTestCase

from telephony_app.emulators.call_panel import CallPanel
from telephony_app.emulators.message_panel import MessagesPanel
from telephony_app.emulators.contacts_panel import ContactsPanel


class TelephonyAppTestCase(AutopilotTestCase, QtIntrospectionTestMixin):
    """A common test case class that provides several useful methods for
    Telephony App tests.

    """
    def setUp(self):
        super(TelephonyAppTestCase, self).setUp()
        self.launch_test_qml()

    def launch_test_qml(self):
            self.app = self.launch_test_application(
                "launch-telephony-app", "--test-contacts")

    def get_main_view_tabs(self):
        """Returns a list of tabs on the main window."""
        return self.app.select_many("TabButton")

    @property
    def call_panel(self):
        return CallPanel(self.app)

    @property
    def messages_panel(self):
        return MessagesPanel(self.app)

    @property
    def contacts_panel(self):
        return ContactsPanel(self.app)
