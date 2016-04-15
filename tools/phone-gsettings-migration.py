#!/usr/bin/python3
# this script is used to migrate existing settings from gsettings to accounts service

import dbus
import sys
import os
import os.path
from pathlib import Path
from gi.repository import Gio

configFile = "accounts-service-migrated"
configPath = os.environ['HOME'] + "/.config/telephony-service"
configFilePath = configPath + "/" + configFile

#create config dir if it does not exist
os.makedirs(configPath, exist_ok=True)

if os.path.isfile(configFilePath):
    sys.exit(0)

# do not allow this script running again
Path(configFilePath).touch()

dict = {}

proxy = dbus.SystemBus().get_object('org.freedesktop.Accounts','/org/freedesktop/Accounts/User%d' % os.getuid())
properties_manager = dbus.Interface(proxy, 'org.freedesktop.DBus.Properties')
currentSimNames = properties_manager.Get('com.ubuntu.touch.AccountsService.Phone', 'SimNames')

print("Migrating gsettings to Accounts Service")

gsettings = Gio.Settings.new('com.ubuntu.phone')
currentSimNames = gsettings.get_value("sim-names")
currentDefaultSimForCalls = gsettings.get_string("default-sim-for-calls")
currentDefaultSimForMessages = gsettings.get_string("default-sim-for-messages")
currentMmsGroupChatEnabled = gsettings.get_boolean("mms-group-chat-enabled")

properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'DefaultSimForCalls', dbus.String(currentDefaultSimForCalls))
properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'DefaultSimForMessages', dbus.String(currentDefaultSimForMessages))
properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'MmsGroupChatEnabled', dbus.Boolean(currentMmsGroupChatEnabled))

if len(currentSimNames) > 0:
    properties_manager.Set('com.ubuntu.touch.AccountsService.Phone', 'SimNames', dbus.Dictionary(currentSimNames))

