/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "farstreamchannel.h"
#include <TelepathyQt/Connection>
#include <TelepathyQt/Farstream/Channel>
#include <farstream/fs-conference.h>
#include <farstream/fs-utils.h>
#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-farstream/channel.h>
#include <gst/gstdebugutils.h>

//#define AUDIO_SOURCE_ELEMENT "autoaudiosrc"
//#define AUDIO_SOURCE_ELEMENT "audiotestsrc"
#define AUDIO_SOURCE_ELEMENT "pulsesrc"

//#define AUDIO_SINK_ELEMENT "autoaudiosink"
//#define AUDIO_SINK_ELEMENT "alsasink"
#define AUDIO_SINK_ELEMENT "pulsesink"

// when defined, incoming audio is teed there
#define AUDIO_OUTPUT_FILE "/tmp/im-output.wav"

#define SINK_GHOST_PAD_NAME "sink"
#define SRC_GHOST_PAD_NAME "src"

class LifetimeTracer {
private:
    const char *filename;
    int line;
    const char *function;
public:
    LifetimeTracer(const char *fn, int l, const char *f): filename(fn), line(l), function(f) { qDebug() << filename << ":" << line << ":" << " entering " << function; }
    ~LifetimeTracer() { qDebug() << filename << ":" << line << ":" << " leaving " << function; }
};
#define LIFETIME_TRACER() LifetimeTracer lifetime_tracer(__FILE__,__LINE__,__PRETTY_FUNCTION__)
#define TRACE() qDebug() << __FILE__ << ":" << __LINE__ << ": trace";

FarstreamChannel::FarstreamChannel(TfChannel *tfChannel, QObject *parent) :
    QObject(parent),
    mTfChannel(tfChannel),
    mState(Tp::MediaStreamStateDisconnected),
    mGstPipeline(0),
    mGstBus(0),
    mGstBusSource(0),
    mGstAudioInput(0),
    mGstAudioInputVolume(0),
    mGstAudioOutput(0),
    mGstAudioOutputVolume(0),
    mGstAudioOutputSink(0),
    mGstAudioOutputActualSink(0)
{
    LIFETIME_TRACER();

    if (!mTfChannel) {
        setError("Unable to create Farstream channel");
        return;
    }

    // connect to the glib-style signals in farstream channel
    mSHClosed = g_signal_connect(mTfChannel, "closed",
        G_CALLBACK(&FarstreamChannel::onClosed), this);
    mSHFsConferenceAdded = g_signal_connect(mTfChannel, "fs-conference-added",
        G_CALLBACK(&FarstreamChannel::onFsConferenceAdded), this);
    mSHFsConferenceRemoved = g_signal_connect(mTfChannel, "fs-conference-removed",
        G_CALLBACK(&FarstreamChannel::onFsConferenceRemoved), this);
    mSHContentAdded = g_signal_connect(mTfChannel, "content-added",
        G_CALLBACK(&FarstreamChannel::onContentAdded), this);
    mSHContentRemoved = g_signal_connect(mTfChannel, "content-removed",
        G_CALLBACK(&FarstreamChannel::onContentRemoved), this);
}

FarstreamChannel::~FarstreamChannel()
{
    LIFETIME_TRACER();

    deinitAudioOutput();
    deinitAudioInput();
    deinitGstreamer();

    if (mTfChannel) {
        if (g_signal_handler_is_connected(mTfChannel, mSHClosed)) {
            g_signal_handler_disconnect(mTfChannel, mSHClosed);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHFsConferenceAdded)) {
            g_signal_handler_disconnect(mTfChannel, mSHFsConferenceAdded);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHFsConferenceRemoved)) {
            g_signal_handler_disconnect(mTfChannel, mSHFsConferenceRemoved);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHContentAdded)) {
            g_signal_handler_disconnect(mTfChannel, mSHContentAdded);
            mSHClosed = 0;
        }
        if (g_signal_handler_is_connected(mTfChannel, mSHContentRemoved)) {
            g_signal_handler_disconnect(mTfChannel, mSHContentRemoved);
            mSHClosed = 0;
        }
        g_object_unref(mTfChannel);
        mTfChannel = 0;
    }
}

Tp::MediaStreamState FarstreamChannel::state() const
{
    return mState;
}

void FarstreamChannel::setState(Tp::MediaStreamState state)
{
    qDebug() << "FarstreamChannel::setState(" << state << ") current mState=" << mState;

    if (mState != state) {
        mState = state;
        Q_EMIT stateChanged();
    }
}

void FarstreamChannel::setError(const QString &errorMessage)
{
    qDebug() << "ERROR: " << errorMessage;

    Q_EMIT error(errorMessage);
}

void FarstreamChannel::init()
{
    LIFETIME_TRACER();
    initGstreamer();

    if (mGstPipeline) {
        GstStateChangeReturn ret = gst_element_set_state(mGstPipeline, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            setError("GStreamer pipeline cannot be played");
            return;
        }
    }

    setState(Tp::MediaStreamStateConnecting);
}

void FarstreamChannel::initGstreamer()
{
    LIFETIME_TRACER();

    mGstPipeline = gst_pipeline_new(NULL);
    if (!mGstPipeline) {
        setError("Gstreamer pipeline could not be created");
        return;
    }

    mGstBus = gst_pipeline_get_bus(GST_PIPELINE(mGstPipeline));
    if (!mGstBus) {
        setError("Gstreamer bus could not be retrieved");
        return;
    }

    mGstBusSource = gst_bus_add_watch(mGstBus, (GstBusFunc) &FarstreamChannel::onBusWatch, this);
    if (!mGstBusSource) {
        setError("Gstreamer bus add watch failed");
        return;
    }
}

void FarstreamChannel::deinitGstreamer()
{
    LIFETIME_TRACER();

    Q_FOREACH(FsElementAddedNotifier *notifier, mFsNotifiers) {
        fs_element_added_notifier_remove(notifier, GST_BIN(mGstPipeline));
        g_object_unref(notifier);
    }
    mFsNotifiers.clear();

    if (mGstBusSource) {
        g_source_remove(mGstBusSource);
        mGstBusSource = 0;
    }

    if (mGstBus) {
        gst_object_unref(mGstBus);
        mGstBus = 0;
    }

    if (mGstPipeline) {
        gst_element_set_state(mGstPipeline, GST_STATE_NULL);
        gst_object_unref(mGstPipeline);
        mGstPipeline = 0;
    }
}

static void setPhoneMediaRole(GstElement *element)
{
    LIFETIME_TRACER();

    GstStructure *props = gst_structure_from_string ("props,media.role=phone", NULL);
    g_object_set(element, "stream-properties", props, NULL);
    gst_structure_free(props);
}

static void releaseGhostPad(GstElement *bin, const char *name, GstElement *sink)
{
    LIFETIME_TRACER();

    // if trying to disconnect while connecting, this will not have been setup yet
    if (!bin)
        return;

    qDebug() << "Releasing ghost pad named " << name << " from bin " << gst_element_get_name(bin) << ", sink " << (sink ? gst_element_get_name(sink) : "<none>");
    if (bin) {
        GstPad *ghostPad = gst_element_get_static_pad(GST_ELEMENT(bin), name);
        if (GST_GHOST_PAD(ghostPad)) {
            GstPad *pad = gst_ghost_pad_get_target(GST_GHOST_PAD(ghostPad));
            if (pad) {
                gst_ghost_pad_set_target(GST_GHOST_PAD(ghostPad), NULL);
                if (sink) {
                    qDebug() << "Releasing request pad under ghost pad";
                    gst_element_release_request_pad(sink, pad);
                }
                gst_object_unref(pad);
            }
            gst_element_remove_pad(GST_ELEMENT(bin), ghostPad);
        }
    }
}

void FarstreamChannel::createGhostPad(GstElement *bin, GstPad *pad, const char *name)
{
    LIFETIME_TRACER();
    qDebug() << "Creating ghost pad named " << name << " for bin " << gst_element_get_name(bin);

    if (!pad) {
        setError("Failed to find pad on which to create ghost pad");
        return;
    }

    GstPad *ghost = gst_ghost_pad_new(name, pad);
    gst_object_unref(pad);
    if (!ghost) {
        setError("GStreamer ghost pad failed");
        return;
    }

    gboolean res = gst_element_add_pad(GST_ELEMENT(bin), ghost);
    if (!res) {
        setError("GStreamer add ghost pad failed");
        return;
    }
}

void FarstreamChannel::initAudioInput()
{
    LIFETIME_TRACER();

    gint input_volume = 255.0;

    if (mGstAudioInput) {
      qDebug() << "Audio input already initialized, doing nothing";
      return;
    }

    mGstAudioInput = gst_parse_bin_from_description (
                          "pulsesrc ! audio/x-raw, rate=8000 ! queue"
                          " ! audioconvert ! audioresample"
                          " ! volume name=input_volume ! audioconvert ",
                          TRUE, NULL);
    if (input_volume >= 0)
    {
        mGstAudioInputVolume = gst_bin_get_by_name (GST_BIN (mGstAudioInput), "input_volume");
        g_debug ("Requested volume is: %i", input_volume);
        g_object_set (mGstAudioInputVolume, "volume", (double)input_volume / 255.0, NULL);
        gst_object_unref (mGstAudioInputVolume);
    }
    addBin(mGstAudioInput);
}

void FarstreamChannel::deinitAudioInput()
{
    LIFETIME_TRACER();

    if (!mGstAudioInput) {
      qDebug() << "Audio input not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstAudioInput, GST_STATE_NULL);

    releaseGhostPad(mGstAudioInput, SRC_GHOST_PAD_NAME, NULL);

    if (mGstAudioInput) {
        gst_element_set_state(mGstAudioInput, GST_STATE_NULL);
        gst_object_unref(mGstAudioInput);
        mGstAudioInput = 0;
    }

    if (mGstAudioInputVolume) {
        gst_object_unref(mGstAudioInputVolume);
        mGstAudioInputVolume = 0;
    }
}

GstElement *FarstreamChannel::pushElement(GstElement *bin, GstElement *&last, const char *factory, bool optional, GstElement **copy, bool checkLink)
{
    LIFETIME_TRACER();
    if (copy)
        *copy = NULL;
    GstElement *e = addElementToBin(bin, last, factory, checkLink);
    if (!e) {
        if (optional) {
            qDebug() << "Failed to create or link optional element " << factory;
        }
        else {
            setError(QString("Failed to create or link element ") + factory);
        }
        return NULL;
    }
    last = e;
    if (copy) {
        gst_object_ref(e);
        *copy = e;
    }
    return e;
}

void FarstreamChannel::writeAudioToFile(GstElement *bin, GstElement *tee)
{
#ifdef AUDIO_OUTPUT_FILE
    LIFETIME_TRACER();
    GstPad *src, *sink;
    GstElement *source, *filesink;

    src = gst_element_get_request_pad(tee, "src%d");
    if (!src) {
        qWarning() << "Failed to get src pad from audio output tee";
        return;
    }
    source = gst_element_factory_make("queue", 0);
    if (!source) {
        qWarning() << "Failed to create queue element";
        return;
    }
    if (!gst_bin_add (GST_BIN (bin), source)) {
        qWarning() << "Failed to add queue to audio file bin";
        return;
    }
    sink = gst_element_get_static_pad (source, "sink");
    if (!sink) {
        qWarning() << "Failed to find static sink pad in queue";
        return;
    }
    if (gst_pad_link (src, sink) != GST_PAD_LINK_OK) {
        qWarning() << "Failed to link audio file tee to queue";
        return;
    }
    pushElement(bin, source, "queue", false, NULL, false);
    pushElement(bin, source, "wavenc", false, NULL, false);
    filesink = pushElement(bin, source, "filesink", false, NULL, false);
    if (filesink == NULL) {
        qWarning() << "Failed to push filesink";
        return;
    }
    g_object_set(filesink, "location", AUDIO_OUTPUT_FILE, NULL);
#else
    (void)bin;
    (void)tee;
#endif
}

void FarstreamChannel::initAudioOutput()
{
    LIFETIME_TRACER();

    if (mGstAudioOutput) {
      qDebug() << "Audio output already initialized, doing nothing";
      return;
    }
}

void FarstreamChannel::deinitAudioOutput()
{
    LIFETIME_TRACER();

    if (!mGstAudioOutput) {
      qDebug() << "Audio output not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstAudioOutput, GST_STATE_NULL);

    releaseGhostPad(mGstAudioOutput, SINK_GHOST_PAD_NAME, NULL);

    if (mGstAudioOutput) {
        gst_element_set_state(mGstAudioOutput, GST_STATE_NULL);
        gst_object_unref(mGstAudioOutput);
        mGstAudioOutput = 0;
    }

    if (mGstAudioOutputVolume) {
        gst_object_unref(mGstAudioOutputVolume);
        mGstAudioOutputVolume = 0;
    }

    if (mGstAudioOutputSink) {
        gst_object_unref(mGstAudioOutputSink);
        mGstAudioOutputSink = 0;
    }

    if (mGstAudioOutputActualSink) {
        gst_object_unref(mGstAudioOutputActualSink);
        mGstAudioOutputActualSink = 0;
    }
}

void FarstreamChannel::onClosed(TfChannel *tfc, FarstreamChannel *self)
{
    LIFETIME_TRACER();
    Q_UNUSED(tfc);

    qDebug() << "FarstreamChannel::onClosed:";
    self->setState(Tp::MediaStreamStateDisconnected);
}

gboolean FarstreamChannel::onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self)
{
    Q_UNUSED(bus);

    if (!self->mTfChannel) {
        return TRUE;
    }

    tf_channel_bus_message (self->mTfChannel, message);

    if (GST_MESSAGE_TYPE (message) == GST_MESSAGE_ERROR) {
        GError *error = NULL;
        gchar *debug = NULL;
        gst_message_parse_error (message, &error, &debug);
        g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (message->src), error->message);
        g_printerr ("Debugging info: %s\n", (debug) ? debug : "none");
        g_error_free (error);
        g_free (debug);
    }

    return TRUE;
}

void FarstreamChannel::setMute(bool mute)
{
    LIFETIME_TRACER();
    qDebug() << "FarstreamChannel::setMute: mute=" << mute;

    if (!mGstAudioInputVolume) {
        return;
    }

    g_object_set(G_OBJECT(mGstAudioInputVolume), "mute", mute, NULL);
}

bool FarstreamChannel::mute() const
{
    LIFETIME_TRACER();
    if (!mGstAudioInputVolume) {
        return false;
    }

    gboolean mute = FALSE;
    g_object_get (G_OBJECT(mGstAudioInputVolume), "mute", &mute, NULL);

    return mute;
}

void FarstreamChannel::setInputVolume(double volume)
{
    LIFETIME_TRACER();
    qDebug() << "FarstreamChannel::setInputVolume: volume=" << volume;

    if (!mGstAudioInputVolume) {
        return;
    }

    GParamSpec *pspec;
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(mGstAudioInputVolume), "volume");
    if (!pspec) {
        return;
    }

    GParamSpecDouble *pspec_double;
    pspec_double = G_PARAM_SPEC_DOUBLE(pspec);
    if (!pspec_double) {
        return;
    }

    volume = CLAMP(volume, pspec_double->minimum, pspec_double->maximum);
    g_object_set(G_OBJECT(mGstAudioInputVolume), "volume", volume, NULL);
}

double FarstreamChannel::inputVolume() const
{
    LIFETIME_TRACER();
    if (!mGstAudioInputVolume) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(mGstAudioInputVolume), "volume", &volume, NULL);

    return volume;
}

void FarstreamChannel::setVolume(double volume)
{
    LIFETIME_TRACER();
    qDebug() << "FarstreamChannel::setVolume: volume=" << volume;

    GstElement *volumeElement = mGstAudioOutputVolume ? mGstAudioOutputVolume : mGstAudioOutputActualSink;
    if (!volumeElement) {
        return;
    }

    GParamSpec *pspec;
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(volumeElement), "volume");
    if (!pspec) {
        return;
    }

    GParamSpecDouble *pspec_double;
    pspec_double = G_PARAM_SPEC_DOUBLE(pspec);
    if (!pspec_double) {
        return;
    }

    volume = CLAMP(volume, pspec_double->minimum, pspec_double->maximum);
    g_object_set(G_OBJECT(volumeElement), "volume", volume, NULL);
}

double FarstreamChannel::volume() const
{
    LIFETIME_TRACER();
    GstElement *volumeElement = mGstAudioOutputVolume ? mGstAudioOutputVolume : mGstAudioOutputActualSink;
    if (!volumeElement) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(volumeElement), "volume", &volume, NULL);

    return volume;
}

void FarstreamChannel::stop()
{
    LIFETIME_TRACER();
  if (mGstPipeline) {
    gst_element_set_state(mGstPipeline, GST_STATE_NULL);
  }
}

GstElement *FarstreamChannel::addElementToBin(GstElement *bin, GstElement *src, const char *factoryName, bool checkLink)
{
    LIFETIME_TRACER();
    qDebug() << "FarstreamChannel::addElementToBin: bin=" << bin << " src=" << src << " factoryName=" << factoryName;

    GstBin *binobj = GST_BIN(bin);
    if (!binobj) {
        setError(QLatin1String("Element factory not found ") + factoryName);
        return 0;
    }

    GstElement *ret;
    if ((ret = gst_element_factory_make(factoryName, 0)) == 0) {
        setError(QLatin1String("Element factory not found ") + factoryName);
        return 0;
    }
    
    return addAndLink(binobj, src, ret, checkLink);
}

GstElement *FarstreamChannel::addAndLink(GstBin *binobj, GstElement *src, GstElement *ret, bool checkLink)
{
    LIFETIME_TRACER();
    gboolean res;

    qDebug() << "FarstreamChannel::addAndLink: binobj="
	     << gst_element_get_name(GST_ELEMENT(binobj)) 
	     << " src="
	     << (src ? gst_element_get_name(src) : "(NULL)")
             << " dst="
	     << gst_element_get_name(ret);

    if (!gst_bin_add(binobj, ret)) {
        setError(QLatin1String("Could not add to bin "));
        gst_object_unref(ret);
        return 0;
    }

    if (!src) {
        return ret;
    }

    if (checkLink) {
      res = gst_element_link(src, ret);
    }
    else {
      res = gst_element_link_pads_full(src, NULL, ret, NULL, GST_PAD_LINK_CHECK_NOTHING);
    }
    if (!res) {
        setError(QLatin1String("Failed to link "));
        gst_bin_remove(binobj, ret);
        return 0;
    }

    return ret;
}

void FarstreamChannel::onFsConferenceAdded(TfChannel *tfc, FsConference * conf, FarstreamChannel *self)
{
    LIFETIME_TRACER();
    Q_UNUSED(tfc);
    Q_ASSERT(conf);

    qDebug() << "FarstreamChannel::onFsConferenceAdded: tfc=" << tfc << " conf=" << conf << " self=" << self;

    if (!self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    /* Add notifier to set the various element properties as needed */
    GKeyFile *keyfile = fs_utils_get_default_element_properties(GST_ELEMENT(conf));
    if (keyfile != NULL)
    {
        qDebug() << "Loaded default codecs for " << GST_ELEMENT_NAME(conf);
        FsElementAddedNotifier *notifier = fs_element_added_notifier_new();
        fs_element_added_notifier_set_properties_from_keyfile(notifier, keyfile);
        fs_element_added_notifier_add (notifier, GST_BIN (self->mGstPipeline));
        self->mFsNotifiers.append(notifier);
    }

    // add conference to the pipeline
    gboolean res = gst_bin_add(GST_BIN(self->mGstPipeline), GST_ELEMENT(conf));
    if (!res) {
        self->setError("GStreamer farstream conference could not be added to the bin");
        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(conf), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer farstream conference cannot be played");
        return;
    }
}

void FarstreamChannel::onFsConferenceRemoved(TfChannel *tfc, FsConference * conf, FarstreamChannel *self)
{
    LIFETIME_TRACER();
    Q_UNUSED(tfc);
    Q_ASSERT(conf);

    qDebug() << "FarstreamChannel::onFsConferenceRemoved: tfc=" << tfc << " conf=" << conf << " self=" << self;

    if (!self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    GstStateChangeReturn ret = gst_element_set_state(GST_ELEMENT(conf), GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer farstream conference cannot be set to null");
        return;
    }

    if (self->mGstPipeline) {
        // remove conference to the pipeline
        gboolean res = gst_bin_remove(GST_BIN(self->mGstPipeline), GST_ELEMENT(conf));
        if (!res) {
            self->setError("GStreamer farstream conference could not be added to the bin");
            return;
        }
    }
}

static const char *get_media_type_string(guint type)
{
  switch (type) {
    case TP_MEDIA_STREAM_TYPE_AUDIO:
      return "audio";
    case TP_MEDIA_STREAM_TYPE_VIDEO:
      return "video";
    default:
      Q_ASSERT(false);
      return "unknown";
  }
}

void FarstreamChannel::addBin(GstElement *bin)
{
    LIFETIME_TRACER();
    if (!bin)
        return;

    gboolean res = gst_bin_add(GST_BIN(mGstPipeline), bin);
    if (!res) {
        setError("GStreamer could not add bin to the pipeline");
        return;
    }

    gst_element_set_locked_state (bin, TRUE);
}

void FarstreamChannel::onContentAdded(TfChannel *tfc, TfContent * content, FarstreamChannel *self)
{
    Q_UNUSED(tfc);

    LIFETIME_TRACER();

    /*
    * Tells the application that a content has been added. In the callback for
    * this signal, the application should set its preferred codecs, and hook
    * up to any signal from #TfContent it cares about. Special care should be
    * made to connect #TfContent::src-pad-added as well
    * as the #TfContent::start-sending and #TfContent::stop-sending signals.
    */

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    if (!content) {
        self->setError("Invalid content received");
        return;
    }

    g_signal_connect(content, "src-pad-added",
                     G_CALLBACK(&FarstreamChannel::onSrcPadAddedContent), self);
    g_signal_connect(content, "start-sending",
                     G_CALLBACK(&FarstreamChannel::onStartSending), self);
    g_signal_connect(content, "stop-sending",
                     G_CALLBACK(&FarstreamChannel::onStopSending), self);

}

void FarstreamChannel::removeBin(GstElement *bin, bool isSink)
{
    LIFETIME_TRACER();
    if (!bin)
        return;

    gst_element_set_locked_state(bin, TRUE);

    if (gst_element_set_state(bin, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
        setError("Failed to stop bin");
        return;
    }

    TRACE();
    GstPad *pad = gst_element_get_static_pad(bin, isSink ? SINK_GHOST_PAD_NAME : SRC_GHOST_PAD_NAME);
    if (!pad) {
        setError("GStreamer get sink element source pad failed");
        return;
    }

    TRACE();
    GstPad *peer = gst_pad_get_peer (pad);
    if (!peer) {
        if (isSink) {
          qDebug() << "Pad has no peer, but it's from a sink which may not have been added, done";
        }
        else {
          setError("Pad has no peer");
        }
    }
    else {
        bool resUnlink = GST_PAD_IS_SRC (pad) ? gst_pad_unlink (pad, peer) : gst_pad_unlink(peer, pad);
        if (!resUnlink) {
            setError("GStreamer could not unlink output bin pad");
            return;
        }
    }

    TRACE();
    gboolean res = gst_bin_remove(GST_BIN(mGstPipeline), bin);
    if (!res) {
        setError("GStreamer could not remove bin from the pipeline");
        return;
    }
    TRACE();
}

void FarstreamChannel::onContentRemoved(TfChannel *tfc, TfContent * content, FarstreamChannel *self)
{
    Q_UNUSED(tfc);
    Q_UNUSED(content);
    Q_UNUSED(self);

    LIFETIME_TRACER();

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return;
    }

    if (!content) {
        self->setError("Invalid content received");
        return;
    }

    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);
    qDebug() << "FarstreamChannel::onContentRemoved: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Audio content removed";
        self->removeBin(self->mGstAudioInput);
        self->removeBin(self->mGstAudioOutput, true);
    } else {
        Q_ASSERT(false);
        return;
    }
}

bool FarstreamChannel::onStartSending(TfContent *content, FarstreamChannel *self)
{
    LIFETIME_TRACER();

    if (!self || !self->mGstPipeline) {
        self->setError("GStreamer pipeline not setup");
        return false;
    }

    if (!content) {
        self->setError("Invalid content received");
        return false;
    }

    guint media_type;
    GstPad *sink;
    g_object_get(content, "media-type", &media_type, "sink-pad", &sink, NULL);
    if (!sink) {
        self->setError("GStreamer cannot get sink pad from content");
        return false;
    }
    qDebug() << "FarstreamChannel::onStartSending: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    GstElement *sourceElement = 0;
    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio sink, initializing audio input";
        if (!self->mGstAudioInput) {
            self->initAudioInput();
        }
        sourceElement = self->mGstAudioInput;
    } else {
        Q_ASSERT(false);
        return false;
    }

    if (!sourceElement) {
        self->setError("GStreamer source element not found");
        return false;
    }

    GstPad *pad = gst_element_get_static_pad(sourceElement, "src");
    if (!pad) {
        self->setError("GStreamer get source element source pad failed");
        return false;
    }

    GstPadLinkReturn resLink = gst_pad_link(pad, sink);
    gst_object_unref(pad);
    gst_object_unref(sink);
    if (resLink != GST_PAD_LINK_OK && resLink != GST_PAD_LINK_WAS_LINKED) {
        self->setError("GStreamer could not link input source pad to sink");
        return false;
    }

    gst_element_set_state(sourceElement, GST_STATE_PLAYING);
    return true;
}

void FarstreamChannel::onStopSending(TfContent *content, FarstreamChannel *self)
{
    LIFETIME_TRACER();

    guint media_type;
    GstPad *sink;
    g_object_get(content, "media-type", &media_type, "sink-pad", &sink, NULL);
    if (!sink) {
        self->setError("GStreamer cannot get sink pad from content");
        return;
    }
    qDebug() << "FarstreamChannel::onStopSending: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    TRACE();
    GstElement *sourceElement = 0;
    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio sink";
        if (!self->mGstAudioInput) {
            qDebug() << "Audio input is not initialized";
            Q_ASSERT(false);
            return;
        }
        sourceElement = self->mGstAudioInput;
    } else {
        Q_ASSERT(false);
    }

    TRACE();
    if (!sourceElement) {
        self->setError("GStreamer source element not found");
        return;
    }

    gst_element_set_locked_state(sourceElement, TRUE);

    TRACE();
    if (gst_element_set_state(sourceElement, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
        self->setError("Failed to stop bin");
        return;
    }

    TRACE();
    GstPad *pad = gst_element_get_static_pad(sourceElement, "src");
    if (!pad) {
        self->setError("GStreamer get source element source pad failed");
        return;
    }

    TRACE();
    bool resUnlink = gst_pad_unlink(pad, sink);
    if (!resUnlink) {
        self->setError("GStreamer could not unlink input source pad from sink");
        return;
    }

    TRACE();
}

void FarstreamChannel::onSrcPadAddedContent(TfContent *content, uint handle, FsStream *stream, GstPad *pad, FsCodec *codec, FarstreamChannel *self)
{
    Q_UNUSED(content);
    Q_UNUSED(handle);
    Q_UNUSED(codec);
    Q_ASSERT(stream);

    LIFETIME_TRACER();

    gchar *cstr = fs_codec_to_string (codec);
      FsMediaType mtype;
      GstPad *sinkpad;
      GstElement *element;
      GstStateChangeReturn ret;

      g_debug ("New src pad: %s", cstr);
      g_object_get (content, "media-type", &mtype, NULL);

      qDebug() << "FarstreamChannel::onSrcPadAddedContent: stream=" << stream << " type=" << mtype << " (" << get_media_type_string(mtype) << ")" << "pad = " << pad;

      switch (mtype)
        {
          case FS_MEDIA_TYPE_AUDIO:
            {
              GstElement *volume = NULL;
              gchar *tmp_str = g_strdup_printf ("audioconvert ! audioresample "
                  "! volume name=\"output_volume%s\" "
                  "! audioconvert ! autoaudiosink", cstr);
              element = gst_parse_bin_from_description (tmp_str,
                  TRUE, NULL);
              g_free (tmp_str);

              tmp_str = g_strdup_printf ("output_volume%s", cstr);
              volume = gst_bin_get_by_name (GST_BIN (element), tmp_str);
              g_free (tmp_str);
              self->mGstAudioOutputVolume = volume;
              g_object_ref(self->mGstAudioOutputVolume);
              break;
            }
      }

      self->addBin(element);
      self->mGstAudioOutput = element;
      g_object_ref(self->mGstAudioOutput);
      sinkpad = gst_element_get_static_pad (element, "sink");
      ret = gst_element_set_state (element, GST_STATE_PLAYING);
      if (ret == GST_STATE_CHANGE_FAILURE)
        {
          g_warning ("Failed to start sink pipeline !?");
          return;
        }

      if (GST_PAD_LINK_FAILED (gst_pad_link (pad, sinkpad)))
        {
          g_warning ("Couldn't link sink pipeline !?");
          return;
        }

      g_object_unref (sinkpad);
}
