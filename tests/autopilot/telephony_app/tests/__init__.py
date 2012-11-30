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
from telephony_app.emulators.communication_panel import CommunicationPanel
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

    def get_main_view(self):
        return self.app.select_single("QQuickView")

    def move_to_next_tab(self):
        main_view = self.get_main_view()
        start_x = main_view.geometry[0] + self.get_main_view().geometry[2] * 0.75
        stop_x = self.get_main_view().geometry[0] + self.get_main_view().geometry[2] * 0.15
        y_line = self.get_main_view().geometry[1] + 10
        self.mouse.move(start_x, y_line)
        self.mouse.press()
        self.mouse.move(stop_x, y_line)
        self.mouse.release()

    @property
    def call_panel(self):
        return CallPanel(self.app)

    @property
    def communication_panel(self):
        return CommunicationPanel(self.app)

    @property
    def contacts_panel(self):
        return ContactsPanel(self.app)
