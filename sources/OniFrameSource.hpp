#pragma once

//#include <memory>

#include <QtCore/qobject.h>

#include <openni2/OpenNI.h>


#include <QtCore/qtimer.h>
#include <QtMultimedia/qvideoframe.h>
//QT_BEGIN_NAMESPACE
//class QVideoFrame;
//QT_END_NAMESPACE


//namespace openni
//{
//class Device;
//class VideoStream;
//}


class OniFrameSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(QVideoFrame::PixelFormat colorPixelFormat READ colorPixelFormat)
    //Q_PROPERTY(QVideoFrame::PixelFormat depthPixelFormat READ depthPixelFormat)

public:
    enum class State
    {
        Stopped, Playing, Paused
    };


    OniFrameSource();
    ~OniFrameSource();

    State state() const;

    int width() const;
    int height() const;
    QVideoFrame::PixelFormat colorPixelFormat() const;

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

private:
   // std::unique_ptr<openni::Device> m_device;
    openni::Device              m_device;
    openni::PlaybackControl*    m_playbackControl = nullptr;
    openni::VideoStream         m_colorStream;
    openni::VideoStream         m_depthStream;

    quint8*                      m_depthFrameBuffer;

    State                       m_state;

    int                         m_width;
    int                         m_height;
    QVideoFrame::PixelFormat    m_colorPixelFormat;

    int                         m_numberOfFrames;
    int                         m_currentFrame;
    int                         m_framerate;
    QTimer                      m_timer;
};
