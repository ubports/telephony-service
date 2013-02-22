# -*- Mode: Python; coding: utf-8; indent-tabs-mode: nil; tab-width: 4 -*-
# Copyright 2012 Canonical
#
# This file is part of telephony-app.
#
# telephony-app is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3, as published
# by the Free Software Foundation.

class Utils(object):
    """Utility functions to write tests for telephony-app"""

    def __init__(self, app):
        self.app = app

    def get_tool_bar(self):
        """Returns the toolbar in the main events view."""
        return self.app.select_single("Toolbar")

    def get_tool_button(self, index):
        """Returns the toolbar button at position `index`"""
        tool_bar = self.get_tool_bar()
        item = tool_bar.get_children_by_type("QQuickItem")[0]
        row = item.get_children_by_type("QQuickRow")[0]
        return row.get_children_by_type("Button")[index]


