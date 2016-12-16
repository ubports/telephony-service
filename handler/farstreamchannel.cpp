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

#include "farstreamchannel.h"
#include <farstream/fs-utils.h>
#include <QDebug>

FarstreamChannel::FarstreamChannel(TfChannel *channel, QObject *parent) :
    mChannel(channel), QObject(parent), mPipeline(0), mBus(0), mBusSource(0),
    mConferenceAddedSignal(0), mConferenceRemovedSignal(0), mContentAddedSignal(0),
    mContentRemovedSignal(0), mAudioInput(0), mAudioOutput(0)
{
    qDebug() << __PRETTY_FUNCTION__;
    initialize();
}

FarstreamChannel::~FarstreamChannel()
{
    // stop audio input and output
    if (mAudioInput) {
        setState(mAudioInput, GST_STATE_NULL);
        gst_object_unref(mAudioInput);
    }

    if (mAudioOutput) {
        setState(mAudioOutput, GST_STATE_NULL);
        gst_object_unref(mAudioOutput);
    }

    // now clear the notifiers
    Q_FOREACH(FsElementAddedNotifier *notifier, mNotifiers) {
        fs_element_added_notifier_remove(notifier, GST_BIN(mPipeline));
        g_object_unref(notifier);
    }
    mNotifiers.clear();

    // clear the bus stuff
    if (mBusSource) {
        g_source_remove(mBusSource);
    }

    if (mBus) {
        gst_object_unref(mBus);
    }

    // and finally the pipeline
    if (mPipeline) {
        setState(mPipeline, GST_STATE_NULL);
        gst_object_unref(mPipeline);
    }
}

void FarstreamChannel::setMute(bool mute)
{
    GstElement *input_volume = gst_bin_get_by_name(GST_BIN(mPipeline), "input_volume");
    g_object_set(input_volume, "mute", mute, NULL);
    g_object_unref(input_volume);
}

void FarstreamChannel::initialize()
{
    qDebug() << __PRETTY_FUNCTION__;
    // connect all the signals
    mConferenceAddedSignal = g_signal_connect(mChannel, "fs-conference-added",
                                              G_CALLBACK(&FarstreamChannel::onConferenceAdded),
                                              this);
    mConferenceRemovedSignal = g_signal_connect(mChannel, "fs-conference-removed",
                                                G_CALLBACK(&FarstreamChannel::onConferenceRemoved),
                                                this);
    mContentAddedSignal = g_signal_connect(mChannel, "content-added",
                                           G_CALLBACK(&FarstreamChannel::onContentAdded),
                                           this);
    mContentRemovedSignal = g_signal_connect(mChannel, "content-removed",
                                             G_CALLBACK(&FarstreamChannel::onContentRemoved),
                                             this);

    // and initialize the gstreamer pipeline
    mPipeline = gst_pipeline_new(NULL);
    if (!mPipeline) {
        qCritical() << "Failed to create GStreamer pipeline.";
        return;
    }

    mBus = gst_pipeline_get_bus(GST_PIPELINE(mPipeline));
    if (!mBus) {
        qCritical() << "Failed to get GStreamer pipeline bus.";
        return;
    }

    mBusSource = gst_bus_add_watch(mBus, (GstBusFunc) &FarstreamChannel::onBusWatch, this);

    if (!setState(mPipeline, GST_STATE_PLAYING)) {
        return;
    }
}

GstElement *FarstreamChannel::initializeAudioSource(TfContent *content)
{
    qDebug() << __PRETTY_FUNCTION__;
    GstElement *element = gst_parse_bin_from_description ("pulsesrc ! audio/x-raw, rate=8000 ! queue"
                                                          " ! audioconvert ! audioresample"
                                                          " ! volume name=input_volume ! audioconvert ",
                                                          TRUE, NULL);
    gint input_volume = 0;
    g_object_get (content, "requested-input-volume", &input_volume, NULL);

    if (input_volume >= 0) {
        GstElement *volume = gst_bin_get_by_name (GST_BIN (element), "input_volume");
        g_object_set (volume, "volume", (double)input_volume / 255.0, NULL);
        gst_object_unref (volume);
    }

    // FIXME: we probably need to handle input volume request changes
    mAudioOutput = element;
    return element;
}

bool FarstreamChannel::addToPipeline(GstElement *element)
{
    qDebug() << __PRETTY_FUNCTION__ << GST_ELEMENT_NAME(element);
    if (!mPipeline) {
        qWarning() << "No gstreamer pipeline found.";
        return false;
    }

    if (!gst_bin_add(GST_BIN(mPipeline), element)) {
        qCritical() << "Failed to add bin" << GST_ELEMENT_NAME(element) << "to pipeline.";
        return false;
    }
    qDebug() << "Succeeded adding to pipeline!";
    return true;
}

void FarstreamChannel::removeFromPipeline(GstElement *element)
{
    qDebug() << __PRETTY_FUNCTION__ << GST_ELEMENT_NAME(element);
    gst_element_set_locked_state(element, TRUE);
    setState(element, GST_STATE_NULL);
    gst_bin_remove (GST_BIN (mPipeline), element);
}

bool FarstreamChannel::setState(GstElement *element, GstState state)
{
    qDebug() << __PRETTY_FUNCTION__ << GST_ELEMENT_NAME(element) << gst_element_state_get_name(state);
    GstStateChangeReturn result = gst_element_set_state(element, state);
    if (result == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Failed to set GStreamer element" << GST_ELEMENT_NAME(element) << "state to" << gst_element_state_get_name(state);
        return false;
    }
    qDebug() << "Succeeded playing!";
    return true;
}

gboolean FarstreamChannel::onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self)
{
    Q_UNUSED(bus)
    if (!self->mChannel) {
        return TRUE;
    }

    // FIXME: maybe we need to do some error handling here?
    tf_channel_bus_message(self->mChannel, message);
    return TRUE;
}

void FarstreamChannel::onConferenceAdded(TfChannel *channel, FsConference *conference, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(channel)

    /* Add notifier to set the various element properties as needed */
    GKeyFile *keyfile = fs_utils_get_default_element_properties (GST_ELEMENT(conference));
    if (keyfile != NULL) {
        qDebug() << "Loaded default properties for" << GST_ELEMENT_NAME(conference);
        FsElementAddedNotifier *notifier = fs_element_added_notifier_new();
        fs_element_added_notifier_set_properties_from_keyfile(notifier, keyfile);
        fs_element_added_notifier_add(notifier, GST_BIN(self->mPipeline));

        // FIXME: right now we are leaking the notifiers, check when to remove them
        self->mNotifiers.append(notifier);
    }

    if (!self->addToPipeline(GST_ELEMENT(conference))) {
        return;
    }

    self->setState(GST_ELEMENT(conference), GST_STATE_PLAYING);
}

void FarstreamChannel::onConferenceRemoved(TfChannel *channel, FsConference *conference, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(channel);

    // just remove the conference from the pipeline
    self->removeFromPipeline(GST_ELEMENT(conference));
}

void FarstreamChannel::onContentAdded(TfChannel *channel, TfContent *content, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_UNUSED(channel)

    g_signal_connect(content, "src-pad-added",
                     G_CALLBACK(&FarstreamChannel::onSrcPadAdded), self);
    g_signal_connect(content, "start-sending",
                     G_CALLBACK(&FarstreamChannel::onStartSending), self);
    g_signal_connect(content, "stop-sending",
                     G_CALLBACK(&FarstreamChannel::onStopSending), self);
}

void FarstreamChannel::onContentRemoved(TfChannel *channel, TfContent *content, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: implement
}

bool FarstreamChannel::onStartSending(TfContent *content, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    GstPad *sinkPad;
    FsMediaType mediaType;
    GstElement *element;

    g_object_get (content, "sink-pad", &sinkPad, "media-type", &mediaType, NULL);

    switch (mediaType) {
    case FS_MEDIA_TYPE_AUDIO:
        element = self->initializeAudioSource(content);
        break;
    // FIXME: add video support
    default:
        qWarning() << "Unsupported media type:" << mediaType;
        g_object_unref(sinkPad);
        return false;
    }

    if (!self->addToPipeline(element)) {
        g_object_unref(sinkPad);
        return false;
    }

    GstPad *sourcePad = gst_element_get_static_pad (element, "src");
    if (GST_PAD_LINK_FAILED (gst_pad_link (sourcePad, sinkPad))) {
        qCritical() << "Failed to link source pad to content's sink pad";
        g_object_unref(sinkPad);
        g_object_unref(sourcePad);
        return false;
    }

    self->setState(element, GST_STATE_PLAYING);
    self->mAudioInput = element;

    g_object_unref(sinkPad);
    g_object_unref(sourcePad);

    qDebug() << "BLABLA generating dot file";
    GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN (self->mPipeline),
         GST_DEBUG_GRAPH_SHOW_ALL, "telephony-service-gst");
    qDebug() << "BLABLA done!";
    return true;
}

void FarstreamChannel::onStopSending(TfContent *content, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: implement
}

void FarstreamChannel::onSrcPadAdded(TfContent *content, uint handle, FsStream *stream, GstPad *pad, FsCodec *codec, FarstreamChannel *self)
{
    qDebug() << __PRETTY_FUNCTION__;
    gchar *codecString = fs_codec_to_string (codec);
    qDebug() << __PRETTY_FUNCTION__ << "Codec:" << codecString;

    FsMediaType mediaType;
    GstElement *element;

    g_object_get (content, "media-type", &mediaType, NULL);

    switch (mediaType) {
    case FS_MEDIA_TYPE_AUDIO: {
         QString outputVolume = QString("output_volume%1").arg(codecString);
         QString description = QString("audioconvert ! audioresample "
                                       "! volume name=\"%1\" "
                                       "! audioconvert ! autoaudiosink").arg(outputVolume);
         element = gst_parse_bin_from_description (description.toUtf8().data(), TRUE, NULL);
         GstElement *volume = gst_bin_get_by_name (GST_BIN (element), outputVolume.toUtf8().data());

         // FIXME: we need to handle volume request changes in the volume element
         gst_object_unref (volume);
         break;
    }
    // FIXME: handle video
    default:
        qWarning() << "Unsupported media type:" << mediaType;
        return;
    }

    if (!self->addToPipeline(element)) {
        return;
    }

    GstPad *sinkPad = gst_element_get_static_pad (element, "sink");
    if (GST_PAD_LINK_FAILED (gst_pad_link (pad, sinkPad))) {
        qCritical() << "Failed to link content's source pad to local sink pad";
    }

    self->setState(element, GST_STATE_PLAYING);

    g_object_unref (sinkPad);
}
