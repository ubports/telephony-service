/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef FARSIGHTCHANNEL_H
#define FARSIGHTCHANNEL_H

#include <gio/gio.h>
#include <glib-object.h>
#include <farstream/fs-conference.h>
#include <farstream/fs-element-added-notifier.h>
#include <farstream/fs-stream.h>
#include <QObject>
//#include <QtGstQmlSink/qmlpaintervideosurface.h>
//#include <QtGstQmlSink/qmlgstvideoitem.h>
#include <TelepathyQt/Constants>
#include <TelepathyQt/Types>
#include <TelepathyQt/Farstream/Channel>
#include <telepathy-farstream/content.h>
//#include <telepathy-farstream/stream.h>

class FarstreamChannel : public QObject
{
    Q_OBJECT

public:
    explicit FarstreamChannel(TfChannel *tfChannel, QObject *parent = 0);
    ~FarstreamChannel();

    /// global initialization
    void init();
    Tp::MediaStreamState state() const;

    void setMute(bool mute);
    bool mute() const;
    void setInputVolume(double volume);
    double inputVolume() const;
    void setVolume(double volume);
    double volume() const;

    //void swapCamera();
    //bool cameraSwappable() const;
    //int currentCamera() const;
    //QString currentCameraDevice() const;
    //int cameraCount() const;

    //bool canSwapVideos() const;
    //void setIncomingVideo(QmlGstVideoItem *item);
    //void setOutgoingVideo(QmlGstVideoItem *item);

    void stop();
    //void onIncomingVideo(bool incoming);

Q_SIGNALS:
    void stateChanged();
    void error(const QString &errorMessage);
    void remoteVideoRender(bool enabled);

public Q_SLOTS:
    // When orientation changes, we must change the videoflip rotation.
    // We support four orientations:
    // 0 - Right Up
    // 1 - Top Up
    // 2 - Left Up
    // 3 - Top Down
    //void onOrientationChanged(uint orientation);

//private Q_SLOTS:
//    void onGstVideoItemDestroyed(QObject *obj);

private:    
    //static QmlPainterVideoSurface *mIncomingSurface;
    //static QmlPainterVideoSurface *mOutgoingSurface;

    TfChannel *mTfChannel;
    Tp::MediaStreamState mState;

    GstElement *mGstPipeline;
    QList<FsElementAddedNotifier *> mFsNotifiers;
    GstBus *mGstBus;
    uint mGstBusSource;
    GstElement *mGstAudioInput;
    GstElement *mGstAudioInputVolume;
    GstElement *mGstAudioOutput;
    GstElement *mGstAudioOutputVolume;
    GstElement *mGstAudioOutputSink;
    GstElement *mGstAudioOutputActualSink;
    //GstElement *mGstVideoInput;
    //GstElement *mGstVideoSource;
    //GstElement *mGstVideoFlip;
    //GstElement *mGstVideoOutput;
    //GstElement *mGstVideoOutputSink;
    //GstElement *mGstVideoTee;
    //GstElement *mGstIncomingVideoSink;
    //GstElement *mGstOutgoingVideoSink;

    //QmlGstVideoItem *mIncomingVideoItem;
    //QmlGstVideoItem *mOutgoingVideoItem;

    //mutable int mCurrentCamera;
    //mutable int mCameraCount;
    //uint mCurrentOrientation;

    // glib signal handlers
    gulong mSHClosed;
    gulong mSHFsConferenceAdded;
    gulong mSHFsConferenceRemoved;
    gulong mSHContentAdded;
    gulong mSHContentRemoved;

    void setState(Tp::MediaStreamState state);
    void setError(const QString &errorMessage);

    /// initialize the common gstreamer stuff like bus, pipeline
    void initGstreamer();
    void deinitGstreamer();
    /// initialize audio input (mic to local audio stream)
    void initAudioInput();
    void deinitAudioInput();
    /// initialize audio output (remote audio stream to speaker)
    void initAudioOutput();
    void deinitAudioOutput();
    /// initialize video input (camera to local video stream + preview window)
    //void initVideoInput();
    //void deinitVideoInput();
    /// initialize video output (remote video stream to window)
    //void initVideoOutput();
    //void deinitVideoOutput();
    ///
    //void initOutgoingVideoWidget();
    //void deinitOutgoingVideoWidget();
    //void initIncomingVideoWidget();
    //void deinitIncomingVideoWidget();

    // glib style signal handlers for GStream
    static void onClosed(TfChannel *tfc, FarstreamChannel *self);
    static gboolean onBusWatch(GstBus *bus, GstMessage *message, FarstreamChannel *self);

    // signals related with call channel stuff
    static void onFsConferenceAdded(TfChannel *tfc, FsConference * conf, FarstreamChannel *self);
    static void onFsConferenceRemoved(TfChannel *tfc, FsConference * conf, FarstreamChannel *self);
    static void onContentAdded(TfChannel *tfc, TfContent * content, FarstreamChannel *self);
    static void onContentRemoved(TfChannel *tfc, TfContent * content, FarstreamChannel *self);
    static bool onStartSending(TfContent *tfc, FarstreamChannel *self);
    static void onStopSending(TfContent *tfc, FarstreamChannel *self);
    static void onSrcPadAddedContent(TfContent *content, uint handle, FsStream *stream, GstPad *src, FsCodec *codec, FarstreamChannel *self);

    GstElement *addElementToBin(GstElement *bin, GstElement *src, const char *factoryName, bool checkLink = true);
    GstElement *addAndLink(GstBin *bin, GstElement *src, GstElement * target, bool checkLink = true);

    // set video flip method, mirrored = UINT_MAX does not change current mirror method
    //void setVideoFlipMethod(uint cameraRotation, uint mirrored = UINT_MAX);
    //uint cameraRotation() const;
    //uint mirrored() const;

    //int countCameras() const;

    GstElement *pushElement(GstElement *bin, GstElement *&last, const char *factory, bool optional = false, GstElement **copy = NULL, bool checkLink = true);
    void writeAudioToFile(GstElement *bin, GstElement *tee);

    void createGhostPad(GstElement *bin, GstPad *pad, const char *name);
    void addBin(GstElement*);
    void removeBin(GstElement *bin, bool isSink = false);
};

#endif // FARSIGHTCHANNEL_H
