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
    Q_PROPERTY(QVideoFrame::PixelFormat pixelFormat READ pixelFormat)

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

    void newFrameAvailable(const QVideoFrame& frame);

private slots:
    void processFrame();

private:
   // std::unique_ptr<openni::Device> m_device;
    openni::Device m_device;
    openni::PlaybackControl* m_playbackControl = nullptr;
    openni::VideoStream m_colorStream;

    State m_state;

    int m_width;
    int m_height;
    QVideoFrame::PixelFormat m_pixelFormat;

    int m_numberOfFrames;
    int m_currentFrame;
    int m_framerate;
    QTimer m_timer;
};
