#!/bin/python
# this script is used to migrate existing settings from gsettings to accounts service

import dbus
import sys
from gi.repository import Gio

dict = {}

proxy = dbus.SystemBus().get_object('org.freedesktop.Accounts','/org/freedesktop/Accounts/User32011')
properties_manager = dbus.Interface(proxy, 'org.freedesktop.DBus.Properties')
currentSimNames = properties_manager.Get('com.ubuntu.touch.AccountsService.Phone', 'SimNames')

# no changes are required
if len(currentSimNames) > 0:
    sys.exit(0)

print("Migrating gsettings to Accounts Service")

gsettings = Gio.Settings.new('com.ubuntu.phone')
currentSimNames = gsettings.get_value("sim-names")
currentDefaultSimForCalls = gsettings.get_value("default-sim-for-calls")
currentDefaultSimForMessages = gsettings.get_value("default-sim-for-calls")
currentMmsGroupChatEnabled = gsettings.get_boolean("mms-group-chat-enabled")

properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'SimNames', dbus.Dictionary(currentSimNames))
properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'DefaultSimForCalls', dbus.String(currentDefaultSimForCalls))
properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'DefaultSimForMessages', dbus.String(currentDefaultSimForMessages))
properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'MmsGroupChatEnabled', dbus.Boolean(currentMmsGroupChatEnabled))
