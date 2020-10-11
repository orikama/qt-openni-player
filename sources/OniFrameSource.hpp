#pragma once

#include <memory>

#include <OpenNI.h>

#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>
#include <QtMultimedia/qvideoframe.h>


class OniFrameSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(QVideoFrame::PixelFormat pixelFormat READ pixelFormat)

public:
    enum class State
    {
        Stopped, Playing, Paused
    };


    OniFrameSource(QObject* parent = nullptr);
    ~OniFrameSource();

    State state() const;

    int width() const;
    int height() const;
    QVideoFrame::PixelFormat pixelFormat() const;

    void loadOniFile(const QString& url);

public slots:
    void pause();
    void play();
    void setPosition(int position);

signals:
    void stateChanged(OniFrameSource::State newState);

    void durationChanged(int duration);
    void positionChanged(int position);

    void newColorFrame(const QVideoFrame& frame);
    void newDepthFrame(const QVideoFrame& frame);

private slots:
    void processFrame();
    void processDepthFrame1();
    void processDepthFrame2();
    void processDepthFrame3();
    void processDepthFrame4();

private:
    openni::Device              m_device;
    openni::PlaybackControl*    m_playbackControl;
    openni::VideoStream         m_colorStream;
    openni::VideoStream         m_depthStream;

    //std::unique_ptr<quint8[]>   m_depthFrameBuffer;
    quint8*                     m_depthFrameBuffer;

    State                       m_state;

    int                         m_width;
    int                         m_height;
    QVideoFrame::PixelFormat    m_pixelFormat;

    int                         m_numberOfFrames;
    int                         m_currentFrame;
    int                         m_framerate;
    QTimer                      m_timer;
};
