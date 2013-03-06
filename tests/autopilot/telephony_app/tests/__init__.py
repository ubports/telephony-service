# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of telephony-app.
#
# telephony-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Telephony App autopilot tests."""

from autopilot.introspection.qt import QtIntrospectionTestMixin
from autopilot.testcase import AutopilotTestCase

from telephony_app.emulators.call_panel import CallPanel
from telephony_app.emulators.communication_panel import CommunicationPanel
from telephony_app.emulators.contacts_panel import ContactsPanel

from testtools.matchers import Equals
from autopilot.matchers import Eventually

import os


class TelephonyAppTestCase(AutopilotTestCase, QtIntrospectionTestMixin):
    """A common test case class that provides several useful methods for
    Telephony App tests.

    """
    def setUp(self):
        super(TelephonyAppTestCase, self).setUp()

        # Lets assume we are installed system wide if this file is somewhere in /usr
        if os.path.realpath(__file__).startswith("/usr/"):
            self.launch_test_installed()
        else:
            self.launch_test_local()

    def launch_test_local(self):
        self.app = self.launch_test_application(
            "../../telephony-app", "--test-contacts")

    def launch_test_installed(self):
        self.app = self.launch_test_application(
               "telephony-app", "--test-contacts")

    def get_main_view(self):
        return self.app.select_single("QQuickView")

    def get_tabs(self):
        return self.app.select_single("Tabs")

    def move_to_next_tab(self):
        main_view = self.get_main_view()
        tabs = self.get_tabs()
        currentTab = tabs.selectedTabIndex
        start_x = main_view.x + main_view.width * 0.85
        stop_x = main_view.x + main_view.width * 0.15
        y_line = main_view.y + main_view.height * 0.5
        self.pointing_device.drag(start_x, y_line, stop_x, y_line)
        self.assertThat(tabs.selectedTabIndex, Eventually(Equals(currentTab + 1)))

    def reveal_toolbar(self):
        main_view = self.get_main_view()
        x_line = main_view.x + main_view.width * 0.5
        start_y = main_view.y + main_view.height - 1
        stop_y = start_y - 200
        self.pointing_device.drag(x_line, start_y, x_line, stop_y)

    @property
    def call_panel(self):
        return CallPanel(self.app)

    @property
    def communication_panel(self):
        return CommunicationPanel(self.app)

    @property
    def contacts_panel(self):
        return ContactsPanel(self.app)
