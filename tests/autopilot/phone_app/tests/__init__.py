# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of phone-app.
#
# phone-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Phone App autopilot tests."""

from autopilot.input import Mouse, Touch, Pointer
from autopilot.matchers import Eventually
from autopilot.platform import model
from autopilot.testcase import AutopilotTestCase
from testtools.matchers import Equals

from phone_app.emulators.call_panel import CallPanel
from phone_app.emulators.communication_panel import CommunicationPanel
from phone_app.emulators.contacts_panel import ContactsPanel

import os

import logging

logger = logging.getLogger(__name__)


class PhoneAppTestCase(AutopilotTestCase):
    """A common test case class that provides several useful methods for
    Phone App tests.

    """

    if model() == 'Desktop':
        scenarios = [
        ('with mouse', dict(input_device_class=Mouse)),
        ]
    else:
        scenarios = [
        ('with touch', dict(input_device_class=Touch)),
        ]

    local_location = "../../src/phone-app"

    def setUp(self):
        self.pointing_device = Pointer(self.input_device_class.create())
        super(PhoneAppTestCase, self).setUp()

        # Lets assume we are installed system wide if this file is somewhere in /usr
        if os.path.exists(self.local_location):
            self.launch_test_local()
        else:
            self.launch_test_installed()

        main_view = self.get_main_view()
        self.assertThat(main_view.visible, Eventually(Equals(True)))

    def launch_test_local(self):
        self.app = self.launch_test_application(
            self.local_location, "--test-contacts", app_type='qt')

    def launch_test_installed(self):
        if model() == 'Desktop':
            self.app = self.launch_test_application(
                "phone-app",
                "--test-contacts")
        else:
            self.app = self.launch_test_application(
               "phone-app", 
               "--test-contacts",
               "--desktop_file_hint=/usr/share/applications/phone-app.desktop",
               app_type='qt')

    def get_main_view(self):
        return self.app.select_single("QQuickView")

    def get_tabs(self):
        return self.app.select_single("Tabs")

    def move_to_next_tab(self, retries=2):
        main_view = self.get_main_view()
        tabs = self.get_tabs()
        currentTab = tabs.selectedTabIndex
        start_x = main_view.x + main_view.width * 0.85
        stop_x = main_view.x + main_view.width * 0.15
        y_line = main_view.y + main_view.height * 0.5
        self.pointing_device.drag(start_x, y_line, stop_x, y_line)

        # This is usually done very early and sometimes the app is
        # painted too slow to make the first swipe grab.
        # As this is just a helper function and not an actual test case
        # lets give it another (max. retries) chance when it fails.
        try:
            self.assertThat(tabs.selectedTabIndex, Eventually(Equals(currentTab + 1)))
        except:
            if retries > 0:
                logger.warning("Failed to switch tab. Retrying...")
                self.move_to_next_tab(retries-1)
            else:
                logger.warning("Failed to switch tab. Giving up... Test may fail!")

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
