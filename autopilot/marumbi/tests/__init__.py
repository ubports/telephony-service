# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Marumbi autopilot tests."""

from autopilot.introspection.qt import QtIntrospectionTestMixin
from autopilot.testcase import AutopilotTestCase

from marumbi.emulators.call_panel import CallPanel
from marumbi.emulators.message_panel import MessagesPanel
from marumbi.emulators.contacts_panel import ContactsPanel


class MarumbiTestCase(AutopilotTestCase, QtIntrospectionTestMixin):
    """A common test case class that provides several useful methods for
    Marumbi tests.

    """

    def setUp(self):
        super(MarumbiTestCase, self).setUp()
        self.launch_test_qml()

    def launch_test_qml(self):
            self.app = self.launch_test_application(
                "telephony-app")

    def get_main_view_tabs(self):
        """Returns a list of tabs on the main window."""
        qdv = self.app.select_single("QDeclarativeView")
        qgs = qdv.select_single("QGraphicsScene")
        qdi = qgs.select_many("QDeclarativeItem")[0]
        qdi_2 = qdi.select_many("QDeclarativeItem")[0]
        tabs = qdi_2.select_single("Tabs").select_many("QDeclarativeRow")[0]
        return tabs.select_many("TabButton")

    @property
    def call_panel(self):
        return CallPanel(self.app)

    @property
    def messages_panel(self):
        return MessagesPanel(self.app)

    @property
    def contacts_panel(self):
        return ContactsPanel(self.app)
