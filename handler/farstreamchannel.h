/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FARSTREAMCHANNEL_H
#define FARSTREAMCHANNEL_H

#include <QObject>
#include <QList>
#include <telepathy-farstream/telepathy-farstream.h>
#include <farstream/fs-conference.h>
#include <farstream/fs-stream.h>
#include <farstream/fs-element-added-notifier.h>

class FarstreamChannel : public QObject
{
    Q_OBJECT
public:
    explicit FarstreamChannel(TfChannel *channel, QObject *parent = 0);
    ~FarstreamChannel();

    void setMute(bool mute);

protected:
    void initialize();
    GstElement *initializeAudioSource(TfContent *content);

    // gstreamer helpers
    bool addToPipeline(GstElement *element);
    void removeFromPipeline(GstElement *bin);
    bool setState(GstElement *element, GstState state);

    // glib signal handlers
    static gboolean onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self);
    static void onConferenceAdded(TfChannel *channel, FsConference *conference, FarstreamChannel *self);
    static void onConferenceRemoved(TfChannel *channel, FsConference *conference, FarstreamChannel *self);
    static void onContentAdded(TfChannel *channel, TfContent * content, FarstreamChannel *self);
    static void onContentRemoved(TfChannel *channel, TfContent * content, FarstreamChannel *self);
    static bool onStartSending(TfContent *content, FarstreamChannel *self);
    static void onStopSending(TfContent *content, FarstreamChannel *self);
    static void onSrcPadAdded(TfContent *content, uint handle, FsStream *stream, GstPad *pad, FsCodec *codec, FarstreamChannel *self);

private:
    TfChannel *mChannel;

    // gstreamer stuff
    GstElement *mPipeline;
    GstBus *mBus;
    uint mBusSource;
    GstElement *mAudioInput;
    GstElement *mAudioOutput;

    // signal IDs
    gulong mConferenceAddedSignal;
    gulong mConferenceRemovedSignal;
    gulong mContentAddedSignal;
    gulong mContentRemovedSignal;

    // farstream stuff
    QList<FsElementAddedNotifier*> mNotifiers;
};

#endif // FARSTREAMCHANNEL_H
