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
//#include <QtGstQmlSink/qmlvideosurfacegstsink.h>
#include <farstream/fs-conference.h>
#include <farstream/fs-utils.h>
#include <telepathy-glib/telepathy-glib.h>
#include <telepathy-farstream/channel.h>
#include <gst/gstdebugutils.h>

//QmlPainterVideoSurface *FarstreamChannel::mIncomingSurface = 0;
//QmlPainterVideoSurface *FarstreamChannel::mOutgoingSurface = 0;

//#define VIDEO_SOURCE_ELEMENT "v4l2src"
//#define VIDEO_SOURCE_ELEMENT "autovideosrc"
//#define VIDEO_SOURCE_ELEMENT "videotestsrc"

//#define AUDIO_SOURCE_ELEMENT "autoaudiosrc"
//#define AUDIO_SOURCE_ELEMENT "audiotestsrc"
#define AUDIO_SOURCE_ELEMENT "pulsesrc"

//#define AUDIO_SINK_ELEMENT "autoaudiosink"
//#define AUDIO_SINK_ELEMENT "alsasink"
#define AUDIO_SINK_ELEMENT "pulsesink"

// when defined, incoming audio is teed there
#define AUDIO_OUTPUT_FILE "/tmp/im-output.wav"

#define COLORSPACE_CONVERT_ELEMENT "ffmpegcolorspace"

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
    //mGstVideoInput(0),
    //mGstVideoSource(0),
    //mGstVideoFlip(0),
    //mGstVideoOutput(0),
    //mGstVideoOutputSink(0),
    //mGstVideoTee(0),
    //mGstIncomingVideoSink(0),
    //mGstOutgoingVideoSink(0),
    //mIncomingVideoItem(0),
    //mOutgoingVideoItem(0),
    //mCurrentCamera(0),
    //mCameraCount(-1),
    //mCurrentOrientation(1) // TopUp is the default orientation of the camera (for lenovo tablet)
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
    //deinitVideoInput();
    //deinitVideoOutput();
    //deinitIncomingVideoWidget();
    //deinitOutgoingVideoWidget();
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

#if 0
    mGstAudioInput = gst_bin_new("audio-input-bin");
    if (!mGstAudioInput) {
        setError("GStreamer audio input bin could not be created");
        return;
    }
    gst_object_ref(mGstAudioInput);
    gst_object_ref_sink(mGstAudioInput);

    GstElement *source = 0;
    source = addElementToBin(mGstAudioInput, source, AUDIO_SOURCE_ELEMENT);
    if (!source) {
        setError("GStreamer audio input source could not be created");
        return;
    }

    if (!strcmp(AUDIO_SOURCE_ELEMENT, "pulsesrc")) {
        g_object_set(source, "buffer-time", (gint64)20000, NULL);
        g_object_set(source, "latency-time", (gint64)10000, NULL);
        //setPhoneMediaRole(source);
    }

    mGstAudioInputVolume = addElementToBin(mGstAudioInput, source, "volume");
    if (!mGstAudioInputVolume) {
        setError("GStreamer audio input volume could not be created");
    } else {
        source = mGstAudioInputVolume;
        gst_object_ref (mGstAudioInputVolume);
    }

    createGhostPad(mGstAudioInput, gst_element_get_static_pad(source, "src"), SRC_GHOST_PAD_NAME);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(mGstAudioInput), GST_DEBUG_GRAPH_SHOW_ALL, "audioinput");
#endif
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

#if 0
    mGstAudioOutputSink = NULL;
    mGstAudioOutput = gst_bin_new("audio-output-bin");
    if (!mGstAudioOutput) {
        setError("GStreamer audio output could not be created");
        return;
    }
    gst_object_ref(mGstAudioOutput);
    gst_object_ref_sink(mGstAudioOutput);

    GstElement *source = 0;
    pushElement(mGstAudioOutput, source, "queue", false, &mGstAudioOutputSink, false);
    //GstElement *tee = pushElement(mGstAudioOutput, source, "tee", false);

    if (strcmp(AUDIO_SINK_ELEMENT, "pulsesink")) {
        pushElement(mGstAudioOutput, source, "audioresample", true, NULL, false);
        pushElement(mGstAudioOutput, source, "volume", true, &mGstAudioOutputVolume, false);
    }
    else {
        mGstAudioOutputVolume = NULL;
    }
    pushElement(mGstAudioOutput, source, AUDIO_SINK_ELEMENT, false, &mGstAudioOutputActualSink ,false);

    if (!mGstAudioOutputSink) {
        mGstAudioOutputSink = mGstAudioOutputActualSink;
        gst_object_ref(mGstAudioOutputSink);
    }

    g_object_set(G_OBJECT(mGstAudioOutput), "async-handling", TRUE, NULL);
    createGhostPad(mGstAudioOutput, gst_element_get_static_pad(mGstAudioOutputSink, "sink"), SINK_GHOST_PAD_NAME);

    if (!strcmp(AUDIO_SINK_ELEMENT, "pulsesink")) {
        g_object_set(mGstAudioOutputActualSink, "buffer-time", (gint64)50000, NULL);
        g_object_set(mGstAudioOutputActualSink, "latency-time", (gint64)25000, NULL);
        //setPhoneMediaRole(mGstAudioOutputActualSink);
    }

    //writeAudioToFile(mGstAudioOutput, tee);

    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(mGstAudioOutput), GST_DEBUG_GRAPH_SHOW_ALL, "audiooutput");

    if (mGstPipeline) {
        GstStateChangeReturn ret = gst_element_set_state(mGstAudioOutput, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE) {
            setError("GStreamer output bin cannot be played");
            return;
        }
    }



    setMute(false);
    setVolume(100);
#endif
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

#if 0
void FarstreamChannel::initVideoInput()
{
    LIFETIME_TRACER();

    if (mGstVideoInput) {
      qDebug() << "Video input already initialized, doing nothing";
      return;
    }

    mGstVideoInput = gst_bin_new("video-input-bin");
    if (!mGstVideoInput) {
        setError("GStreamer video input could not be created");
        return;
    }
    gst_object_ref(mGstVideoInput);
    gst_object_sink(mGstVideoInput);

    GstElement *element = 0;
    GstElement *source = 0;

    pushElement(mGstVideoInput, source, VIDEO_SOURCE_ELEMENT, false, &mGstVideoSource, false);

    /* Prefer videomaxrate if it's available */
    if (pushElement(mGstVideoInput, source, "videomaxrate", true, NULL, false) == NULL)
        pushElement(mGstVideoInput, source, "fsvideoanyrate", true, NULL, false);
    pushElement(mGstVideoInput, source, "videoscale", false, NULL, false);
    pushElement(mGstVideoInput, source, COLORSPACE_CONVERT_ELEMENT, false, NULL, false);
    pushElement(mGstVideoInput, source, "videoflip", true, &mGstVideoFlip, false);
    if (mGstVideoFlip) {
        // setup video flip element according to current orientation
        onOrientationChanged(mCurrentOrientation);
    }

    GstElement *capsfilter = pushElement(mGstVideoInput, source, "capsfilter", false, NULL, false);
    if (capsfilter) {
        GstCaps *caps = gst_caps_new_simple(
            "video/x-raw-yuv",
            "width", G_TYPE_INT, 320,
            "height", G_TYPE_INT, 240,
            "framerate", GST_TYPE_FRACTION, 15, 1,
            "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC('I','4','2','0'),
            NULL);
        /*
        GstCaps *caps = gst_caps_new_simple(
            "video/x-raw-yuv",
            "width", GST_TYPE_INT_RANGE, 320, 352,
            "height", GST_TYPE_INT_RANGE, 240, 288,
            "framerate", GST_TYPE_FRACTION, 15, 1,
            NULL);
        */
        if (caps) {
            g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
        }
    }

    pushElement(mGstVideoInput, source, "tee", false, &mGstVideoTee, false);
    pushElement(mGstVideoInput, source, "videoscale", false, NULL, false);
    pushElement(mGstVideoInput, source, COLORSPACE_CONVERT_ELEMENT, false, NULL, false);

    element = mGstOutgoingVideoSink; // = GST_ELEMENT(QmlVideoSurfaceGstSink::createSink(mOutgoingSurface));
    element = addAndLink(GST_BIN(mGstVideoInput), source, element, false);
    if (!element) {
        setError("GStreamer outgoing video sink could not be created");
    } else {
        source = element;
        gst_object_ref(mGstOutgoingVideoSink);
        g_object_set(G_OBJECT(element), "async", true, "sync", false, NULL); //, "force-aspect-ratio", true, NULL);
    }

    createGhostPad(mGstVideoInput, gst_element_get_request_pad(mGstVideoTee, "src%d"), SRC_GHOST_PAD_NAME); 
}

void FarstreamChannel::deinitVideoInput()
{
    LIFETIME_TRACER();

    if (!mGstVideoInput) {
      qDebug() << "Video input not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstVideoInput, GST_STATE_NULL);

    releaseGhostPad(mGstVideoInput, SRC_GHOST_PAD_NAME, mGstVideoTee);

    if (mGstVideoTee) {
        gst_object_unref(mGstVideoTee);
        mGstVideoTee = 0;
    }

    if (mGstVideoSource) {
        gst_object_unref(mGstVideoSource);
        mGstVideoSource = 0;
    }

    if (mGstVideoFlip) {
        gst_object_unref(mGstVideoFlip);
        mGstVideoFlip = 0;
    }

    if (mGstOutgoingVideoSink) {
        gst_object_unref(mGstOutgoingVideoSink);
    }

    if (mGstVideoInput) {
        gst_element_set_state(mGstVideoInput, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(mGstVideoInput), mGstOutgoingVideoSink);
        gst_object_unref(mGstVideoInput);
        mGstVideoInput = 0;
    }
}

void FarstreamChannel::initVideoOutput()
{
    LIFETIME_TRACER();

    if (mGstVideoOutput) {
      qDebug() << "Video output already initialized, doing nothing";
      return;
    }

    mGstVideoOutput = gst_bin_new("video-output-bin");
    if (!mGstVideoOutput) {
        setError("GStreamer video output could not be created");
        return;
    }
    gst_object_ref(mGstVideoOutput);
    gst_object_sink(mGstVideoOutput);

    GstElement *source = 0;

    mGstVideoOutputSink = pushElement(mGstVideoOutput, source, "fsfunnel", false, &mGstVideoOutputSink, false);
    pushElement(mGstVideoOutput, source, "videoscale", true, NULL, false);
    pushElement(mGstVideoOutput, source, COLORSPACE_CONVERT_ELEMENT, true, NULL, false);

    /*
    if (!mIncomingSurface) {
        mIncomingSurface = new QmlPainterVideoSurface();
        if (!mIncomingSurface) {
            setError("Unable to create incoming painter video surface");
        }
        //QGLContext* context = new QGLContext(g->context()->format());
        QGLContext* context = new QGLContext(QGLFormat::defaultFormat());
        mIncomingSurface->setGLContext(context);
    }*/

    GstElement *element = mGstIncomingVideoSink; // = GST_ELEMENT(QmlVideoSurfaceGstSink::createSink(mIncomingSurface));
    element = addAndLink(GST_BIN(mGstVideoOutput), source, element, false);
    if (!element) {
        setError("GStreamer qt video sink could not be created");
    } else {
        source = element;
        gst_object_ref(mGstIncomingVideoSink);
        //g_object_set(G_OBJECT(element), "async", false, "sync", false, "force-aspect-ratio", true, NULL);
    }

    createGhostPad(mGstVideoOutput, gst_element_get_request_pad(mGstVideoOutputSink, "sink%d"), SINK_GHOST_PAD_NAME);
}

void FarstreamChannel::deinitVideoOutput()
{
    LIFETIME_TRACER();

    if (!mGstVideoOutput) {
      qDebug() << "Video output not initialized, doing nothing";
      return;
    }

    gst_element_set_state(mGstVideoOutput, GST_STATE_NULL);

    releaseGhostPad(mGstVideoOutput, SINK_GHOST_PAD_NAME, mGstVideoOutputSink);

    if (mGstVideoOutputSink) {
        gst_object_unref(mGstVideoOutputSink);
        mGstVideoOutputSink = 0;
    }

    if (mGstIncomingVideoSink) {
        gst_object_unref(mGstIncomingVideoSink);
    }

    if (mGstVideoOutput) {
        gst_element_set_state(mGstVideoOutput, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(mGstVideoOutput), mGstIncomingVideoSink);
        gst_object_unref(mGstVideoOutput);
        mGstVideoOutput = 0;
    }
}

void FarstreamChannel::initOutgoingVideoWidget()
{
    LIFETIME_TRACER();

    if (mOutgoingVideoItem) {
        mOutgoingVideoItem->setSurface(mOutgoingSurface);
    } else {
        setError("Outgoing video item is unknown yet");
    }
}

void FarstreamChannel::deinitOutgoingVideoWidget()
{
    LIFETIME_TRACER();

    if (mOutgoingVideoItem) {
        mOutgoingVideoItem->setSurface(0);
    }
}

void FarstreamChannel::initIncomingVideoWidget()
{
    LIFETIME_TRACER();

    if (mIncomingVideoItem) {
        mIncomingVideoItem->setSurface(mIncomingSurface);
    } else {
        setError("Incoming video item is unknown yet");
    }
}

void FarstreamChannel::deinitIncomingVideoWidget()
{
    LIFETIME_TRACER();

    if (mIncomingVideoItem) {
        mIncomingVideoItem->setSurface(0);
    }
}
#endif

void FarstreamChannel::onClosed(TfChannel *tfc, FarstreamChannel *self)
{
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

    const GstStructure *s = gst_message_get_structure(message);
    if (s == NULL) {
        goto error;
    }

    if (gst_structure_has_name (s, "farsight-send-codec-changed")) {

        const GValue *val = gst_structure_get_value(s, "codec");
        if (!val) {
            goto error;
        }

        FsCodec *codec = static_cast<FsCodec *> (g_value_get_boxed(val));
        if (!codec) {
            goto error;
        }

        val = gst_structure_get_value(s, "session");
        if (!val) {
            goto error;
        }

        FsSession *session = static_cast<FsSession *> (g_value_get_object(val));
        if (!session) {
            goto error;
        }

        FsMediaType type;
        g_object_get(session, "media-type", &type, NULL);

        gchar *codec_string = fs_codec_to_string(codec);
        qDebug() << "FarstreamChannel::onBusWatch: farsight-send-codec-changed "
                 << " type=" << type
                 << " codec=" << codec_string;
        g_free(codec_string);

    } else if (gst_structure_has_name(s, "farsight-recv-codecs-changed")) {

        const GValue *val = gst_structure_get_value(s, "codecs");
        if (!val) {
            goto error;
        }

        GList *codecs = static_cast<GList *> (g_value_get_boxed(val));
        if (!codecs) {
            goto error;
        }

        val = gst_structure_get_value(s, "stream");
        if (!val) {
            goto error;
        }

        FsStream *stream = static_cast<FsStream *> (g_value_get_object(val));
        if (!stream) {
            goto error;
        }

        FsSession *session = 0;
        g_object_get(stream, "session", &session, NULL);
        if (!session) {
            goto error;
        }

        FsMediaType type = FS_MEDIA_TYPE_LAST;
        g_object_get(session, "media-type", &type, NULL);

        qDebug() << "FarstreamChannel::onBusWatch: farsight-recv-codecs-changed "
                 << " type=" << type;
        GList *list;
        for (list = codecs; list != NULL; list = g_list_next(list)) {
            FsCodec *codec = static_cast<FsCodec *> (list->data);
            gchar *codec_string = fs_codec_to_string(codec);
            qDebug() << "       codec " << codec_string;
            g_free(codec_string);
        }

        g_object_unref(session);

    } else if (gst_structure_has_name(s, "farsight-new-active-candidate-pair")) {

        const GValue *val = gst_structure_get_value(s, "remote-candidate");
        if (!val) {
            goto error;
        }

        FsCandidate *remote_candidate = static_cast<FsCandidate *> (g_value_get_boxed(val));
        if (!remote_candidate) {
            goto error;
        }

        val = gst_structure_get_value(s, "local-candidate");
        if (!val) {
            goto error;
        }

        FsCandidate *local_candidate = static_cast<FsCandidate *> (g_value_get_boxed(val));
        if (!local_candidate) {
            goto error;
        }

        val = gst_structure_get_value(s, "stream");
        if (!val) {
            goto error;
        }

        FsStream *stream = static_cast<FsStream *> (g_value_get_object(val));
        if (!stream) {
            goto error;
        }

        FsSession *session = 0;
        g_object_get(stream, "session", &session, NULL);
        if (!session) {
            goto error;
        }

        FsMediaType type = FS_MEDIA_TYPE_LAST;
        g_object_get(session, "media-type", &type, NULL);

        qDebug() << "FarstreamChannel::onBusWatch: farsight-new-active-candidate-pair ";

        if (remote_candidate) {
            qDebug() << "   remote candidate mediatype=" << type
                     << " foundation=" << remote_candidate->foundation << " id=" << remote_candidate->component_id
                     << " IP=" << remote_candidate->ip << ":" << remote_candidate->port
                     << " BaseIP=" << remote_candidate->base_ip << ":" << remote_candidate->base_port
                     << " proto=" << remote_candidate->proto
                     << " type=" << remote_candidate->type;
        }

        if (local_candidate) {
            qDebug() << "   local candidate mediatype=" << type
                     << " foundation=" << local_candidate->foundation << " id=" << local_candidate->component_id
                     << " IP=" << local_candidate->ip << ":" << local_candidate->port
                     << " BaseIP=" << local_candidate->base_ip << ":" << local_candidate->base_port
                     << " proto=" << local_candidate->proto
                     << " type=" << local_candidate->type;
        }

        g_object_unref(session);
    }

error:

    tf_channel_bus_message(self->mTfChannel, message);
    return TRUE;
}

void FarstreamChannel::setMute(bool mute)
{
    qDebug() << "FarstreamChannel::setMute: mute=" << mute;

    if (!mGstAudioInputVolume) {
        return;
    }

    g_object_set(G_OBJECT(mGstAudioInputVolume), "mute", mute, NULL);
}

bool FarstreamChannel::mute() const
{
    if (!mGstAudioInputVolume) {
        return false;
    }

    gboolean mute = FALSE;
    g_object_get (G_OBJECT(mGstAudioInputVolume), "mute", &mute, NULL);

    return mute;
}

void FarstreamChannel::setInputVolume(double volume)
{
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
    if (!mGstAudioInputVolume) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(mGstAudioInputVolume), "volume", &volume, NULL);

    return volume;
}

void FarstreamChannel::setVolume(double volume)
{
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
    GstElement *volumeElement = mGstAudioOutputVolume ? mGstAudioOutputVolume : mGstAudioOutputActualSink;
    if (!volumeElement) {
        return 0.0;
    }

    double volume;
    g_object_get(G_OBJECT(volumeElement), "volume", &volume, NULL);

    return volume;
}

#if 0
void FarstreamChannel::setIncomingVideo(QmlGstVideoItem *item)
{
    qDebug() << "FarstreamChannel::setIncomingVideo: this=" << (void *) this << " item=" << (void *) item;
    mIncomingVideoItem = item;
    if (item) {
        connect(item, SIGNAL(destroyed(QObject*)), SLOT(onGstVideoItemDestroyed(QObject*)));
    }
    initIncomingVideoWidget();
}

void FarstreamChannel::setOutgoingVideo(QmlGstVideoItem *item)
{
    qDebug() << "FarstreamChannel::setOutgoingVideo: this=" << (void *) this << " item=" << (void *) item;
    mOutgoingVideoItem = item;
    if (item) {
        connect(item, SIGNAL(destroyed(QObject*)), SLOT(onGstVideoItemDestroyed(QObject*)));
    }
    initOutgoingVideoWidget();
}
#endif

void FarstreamChannel::stop()
{
  if (mGstPipeline) {
    gst_element_set_state(mGstPipeline, GST_STATE_NULL);
  }
}

#if 0
void FarstreamChannel::onIncomingVideo(bool incoming)
{
    /* We don't get any notification from farstream about disconnection of streams,
       so we'd just stay at the latest sent frame, waiting for more. So we make sure
       we clear the sink when we stop getting video. */
    if (!mGstPipeline) {
        setError("GStreamer pipeline not setup");
        return;
    }

    if (!incoming && mGstVideoOutput && mGstIncomingVideoSink && mIncomingVideoItem) {
        mIncomingVideoItem->forceIdle();
    }
}

bool FarstreamChannel::canSwapVideos() const
{
    return (mGstVideoOutput != NULL);
}

void FarstreamChannel::onGstVideoItemDestroyed(QObject *obj)
{
    qDebug() << "FarstreamChannel::onGstVideoItemDestroyed: " << obj;

    if (obj == mOutgoingVideoItem) {
        qDebug() << "FarstreamChannel::onGstVideoItemDestroyed: reset outgoing video item";
        mOutgoingVideoItem = 0;
    }

    if (obj == mIncomingVideoItem) {
        qDebug() << "FarstreamChannel::onGstVideoItemDestroyed: reset incoming video item";
        mIncomingVideoItem = 0;
    }
}
#endif

GstElement *FarstreamChannel::addElementToBin(GstElement *bin, GstElement *src, const char *factoryName, bool checkLink)
{
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

#if 0
void FarstreamChannel::swapCamera()
{
    mCurrentCamera++;
    if (mCurrentCamera >= cameraCount()) {
        mCurrentCamera = 0;
    }

    if (mGstVideoInput && mGstVideoSource) {
        GstState state;
        GstState pending;
        gst_element_get_state(mGstVideoSource, &state, &pending, 0);
        qDebug() << "FarstreamChannel::swapCamera:: state=" << state << " pending=" << pending;

        GstStateChangeReturn ret = gst_element_set_state(mGstVideoInput, GST_STATE_PAUSED);
        qDebug() << "FarstreamChannel::swapCamera: camera paused " << ret;

        g_object_set(G_OBJECT(mGstVideoSource), "device", currentCameraDevice().data(), NULL);
        qDebug() << "FarstreamChannel::swapCamera: changing device to " << currentCameraDevice();

        ret = gst_element_set_state(mGstVideoInput, state);
        qDebug() << "FarstreamChannel::swapCamera: camera playing " << ret;
    }
}

bool FarstreamChannel::cameraSwappable() const
{
    return (cameraCount() > 1);
}

int FarstreamChannel::currentCamera() const
{
    return mCurrentCamera;
}

QString FarstreamChannel::currentCameraDevice() const
{
    QString filename("/dev/video");
    return filename + QString::number(mCurrentCamera);
}

int FarstreamChannel::countCameras() const
{
    int count = 0;
    QString filename("/dev/video");
    while (QFile::exists(filename + QString::number(count))) {
        count++;
    }
    return count;
}

int FarstreamChannel::cameraCount() const
{
    if (mCameraCount < 0) {
        mCameraCount = countCameras();
        mCurrentCamera = 0;
    }
    return mCameraCount;
}

void FarstreamChannel::setVideoFlipMethod(uint deg, uint mirrored)
{
    if (!mGstVideoFlip) {
        return;
    }

    if (mirrored == UINT_MAX) {
        mirrored = this->mirrored();
    }

    uint method = 0;
    switch (deg % 360) {
    case 0:
        // 0 : GST_VIDEO_FLIP_METHOD_IDENTITY
        // 4 : GST_VIDEO_FLIP_METHOD_HORIZ,
        method = mirrored ? 4 : 0;
        break;
    case 90:
        // 1 : GST_VIDEO_FLIP_METHOD_90R
        // 5 : GST_VIDEO_FLIP_METHOD_VERT,
        method = mirrored ? 5 : 1;
        break;
    case 180:
        // 2 : GST_VIDEO_FLIP_METHOD_180
        // 6 : GST_VIDEO_FLIP_METHOD_TRANS
        method = mirrored ? 6 : 2;
        break;
    case 270:
        // 3 : GST_VIDEO_FLIP_METHOD_90L
        // 7 : GST_VIDEO_FLIP_METHOD_OTHER
        method = mirrored ? 7 : 3;
        break;
    default:
        qWarning() << "FarstreamChannel::setVideoFlipMethod: invalid rotation";
        return;
    }

    qDebug() << "FarstreamChannel::setVideoFlipMethod: deg=" << deg << "mirrored=" << mirrored << "method=" << method;

    g_object_set(G_OBJECT(mGstVideoFlip), "method", method, NULL);
}

uint FarstreamChannel::cameraRotation() const
{
    if (!mGstVideoFlip) {
        return 0;
    }

    uint method = 0;
    g_object_get(G_OBJECT(mGstVideoFlip), "method", &method, NULL);
    return (method % 4) * 90;
}

uint FarstreamChannel::mirrored() const
{
    if (!mGstVideoFlip) {
        return 0;
    }

    /*
    uint method = 0;
    g_object_get(G_OBJECT(mGstVideoFlip), "method", &method, NULL);
    return (method >= 4) ? 1 : 0;
    */
    return 0;
}

void FarstreamChannel::onOrientationChanged(uint orientation)
{
    qDebug() << "FarstreamChannel::onOrientationChanged: orientation:" << orientation;

    mCurrentOrientation = orientation;
    if (!mGstVideoFlip) {
        return;
    }

    uint rotation = 0;
    switch (orientation) {
    case 0: // Right Up
        rotation = 90;
        break;
    case 1: // Top Up
        rotation = 0;
        break;
    case 2: // Left Up
        rotation = 270;
        break;
    case 3: // Top Down
        rotation = 180;
        break;
    }

    setVideoFlipMethod(rotation);
}
#endif

void FarstreamChannel::onFsConferenceAdded(TfChannel *tfc, FsConference * conf, FarstreamChannel *self)
{
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

#if 0
    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);
    qDebug() << "FarstreamChannel::onContentAdded: content=" << content << " type=" << media_type << "(" << get_media_type_string(media_type) << ")";

    FsSession *session;
    g_object_get(content, "fs-session", &session, NULL);
    qDebug() << "BLABLA session is " << session;
    if (session) {

        FsCodec *codec;
        g_object_get(session, "current-send-codec", &codec, NULL);
        qDebug() << "BLABLA codec is " << codec;
        if (codec) {
            qDebug() << "FarstreamChannel::onStreamCreated: current send codec=" << fs_codec_to_string(codec);
        }


        FsStream *fs_stream = 0;
        g_object_get(content, "fs-stream", &fs_stream, NULL);
        if (fs_stream) {
            GList *codecs;
            g_object_get(fs_stream, "current-recv-codecs", &codecs, NULL);
            if (codecs) {
                qDebug() << " current received codecs: ";
                GList *list;
                for (list = codecs; list != NULL; list = g_list_next (list)) {
                    FsCodec *codec = static_cast<FsCodec *> (list->data);
                    qDebug() << "       codec " << fs_codec_to_string(codec);
                }

                fs_codec_list_destroy(codecs);
            }
        }
    }

    if (media_type == TP_MEDIA_STREAM_TYPE_AUDIO) {
        qDebug() << "Got audio content, adding audio bins";
        self->initAudioInput();
        self->initAudioOutput();
        self->addBin(self->mGstAudioInput);
    } /*else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        qDebug() << "Got video content, adding video bins";
        //self->initOutgoingVideoWidget();
        //self->initVideoInput();
        //self->initIncomingVideoWidget();
        //self->initVideoOutput();
        self->addBin(self->mGstVideoInput);
    }*/ else {
        Q_ASSERT(false);
        return;
    }
#endif

}

void FarstreamChannel::removeBin(GstElement *bin, bool isSink)
{
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
    /*} else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        qDebug() << "Video content removed";
        self->removeBin(self->mGstVideoInput);
        self->removeBin(self->mGstVideoOutput, true);*/
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
    /*} else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        qDebug() << "Got video sink, initializing video intput";
        if (!self->mGstVideoInput) {
            self->initOutgoingVideoWidget();
            self->initVideoInput();
        }
        sourceElement = self->mGstVideoInput;*/
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
    /*} else if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        qDebug() << "Got video sink";
        if (!self->mGstVideoInput) {
            qDebug() << "Video input is not initialized";
            Q_ASSERT(false);
            return;
        }
        sourceElement = self->mGstVideoInput;*/
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
      self->setVolume(1);

#if 0
    guint media_type;
    g_object_get(content, "media-type", &media_type, NULL);

    qDebug() << "FarstreamChannel::onSrcPadAddedContent: stream=" << stream << " type=" << media_type << " (" << get_media_type_string(media_type) << ")" << "pad = " << src;

    GstElement *bin = 0;
    GstPad *pad = 0;

    switch (media_type) {
    case TP_MEDIA_STREAM_TYPE_AUDIO:
        bin = self->mGstAudioOutput;
        break;
    /*case TP_MEDIA_STREAM_TYPE_VIDEO:
        bin = self->mGstVideoOutput;
        break;*/
    default:
        Q_ASSERT(false);
    }

    // Add sink bin the first time we get incoming data
    if (!gst_object_has_ancestor(GST_OBJECT(bin), GST_OBJECT(self->mGstPipeline))) {
        self->addBin(bin);
    }

    pad = gst_element_get_static_pad(bin, SINK_GHOST_PAD_NAME);
    if (!pad) {
        self->setError("Could not find ghost sink pad in bin");
        return;
    }

    /* We can get src-pad-added multiple times without being aware the stream
       might have stopped in the meantime */
    if (gst_pad_is_linked(pad)) {
        gst_element_set_locked_state(bin, TRUE);
        gst_element_set_state (bin, GST_STATE_READY);
        gst_pad_unlink (gst_pad_get_peer(pad), pad);
    }

    GstPadLinkReturn resLink = gst_pad_link(src, pad);
    if (resLink != GST_PAD_LINK_OK && resLink != GST_PAD_LINK_WAS_LINKED) {
        //tf_content_error(content, TP_MEDIA_STREAM_ERROR_MEDIA_ERROR, "Could not link sink");
        self->setError("GStreamer could not link sink pad to source");
        return;
    }

    gst_element_set_locked_state (bin, FALSE);
    if (gst_element_set_state (bin, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        self->setError("GStreamer could not set output bin state to PLAYING");
        return;
    }

    self->setState(Tp::MediaStreamStateConnected);

    /*if (media_type == TP_MEDIA_STREAM_TYPE_VIDEO) {
        Q_EMIT self->remoteVideoRender(true);
    }*/

    // todo If no sink could be linked, try to add fakesink to prevent the whole call

    //GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(self->mGstPipeline), GST_DEBUG_GRAPH_SHOW_ALL, "impipeline2");
#endif
}

