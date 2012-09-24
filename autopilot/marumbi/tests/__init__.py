# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

"""Marumbi autopilot tests."""

from autopilot.introspection.qt import QtIntrospectionTestMixin
from autopilot.testcase import AutopilotTestCase


class MarumbiTestCase(AutopilotTestCase, QtIntrospectionTestMixin):

    """A common test case class that provides several useful methods for Marumbi tests."""

    def setUp(self):
        super(MarumbiTestCase, self).setUp()
        self.launch_test_qml()

    def launch_test_qml(self):
            self.app = self.launch_test_application(
                "telephony-app",
                "-testability")

    def get_main_view_tabs(self):
        """Returns a list of tabs on the main window."""
        qdv_qgs = self.app.select_single("QDeclarativeView").select_single("QGraphicsScene")
        qdi = qdv_qgs.select_many("QDeclarativeItem")[0].select_many("QDeclarativeItem")[0]
        tabs = qdi.select_single("Tabs").select_many("QDeclarativeRow")[0]
        return tabs.select_many("TabButton")
