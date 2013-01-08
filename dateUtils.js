/*
 * Copyright 2012 Canonical Ltd.
 *
 * This file is part of telephony-app.
 *
 * telephony-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

.pragma library

function areSameDay(date1, date2) {
    return date1.getFullYear() == date2.getFullYear()
        && date1.getMonth() == date2.getMonth()
        && date1.getDate() == date2.getDate()
}

function formatLogDate(timestamp) {
    var today = new Date()
    var date = new Date(timestamp)
    if (areSameDay(today, date)) {
        return Qt.formatTime(timestamp, Qt.DefaultLocaleShortDate)
    } else {
        return Qt.formatDateTime(timestamp, Qt.DefaultLocaleShortDate)
    }
}

function friendlyDay(timestamp) {
    var date = new Date(timestamp);
    var today = new Date();
    var yesterday = new Date();
    yesterday.setDate(today.getDate()-1);
    if (areSameDay(today, date)) {
        return "Today";
    } else if (areSameDay(yesterday, date)) {
        return "Yesterday";
    } else {
        return Qt.formatDate(date, Qt.DefaultLocaleShortDate);
    }
}

function formatFriendlyDate(timestamp) {
    return Qt.formatTime(timestamp, Qt.DefaultLocaleShortDate) + " - " + friendlyDay(timestamp);
}

function formatFriendlyCallDuration(duration) {
    var time = new Date(duration);
    var text = "";

    var hours = time.getHours();
    var minutes = time.getMinutes();
    var seconds = time.getSeconds();

    // FIXME: change this function when we get i18n in the app
    if (hours > 0) {
        if (hours == 1) {
            text = "1 hour";
        } else {
            text = hours + " hours";
        }
    } else if (minutes > 0) {
        if (minutes == 1) {
            text = "1 minute";
        } else {
            text = minutes + " minutes";
        }
    } else {
        if (seconds == 1) {
            text = "1 second";
        } else {
            text = seconds + " seconds";
        }
    }

    return text;
}
