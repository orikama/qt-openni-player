#pragma once

#include <memory>
#include <utility>

#include <OpenNI.h>

#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>
#include <QtMultimedia/qvideoframe.h>


class OniFrameSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int colorFrameWidth READ colorFrameWidth)
    Q_PROPERTY(int colorFrameHeight READ colorFrameHeight)
    Q_PROPERTY(int depthFrameWidth READ depthFrameWidth)
    Q_PROPERTY(int depthFrameHeight READ depthFrameHeight)
    Q_PROPERTY(State state READ state)
    Q_PROPERTY(QVideoFrame::PixelFormat pixelFormat READ pixelFormat)
    //Q_PROPERTY(DepthMode depthMode WRITE setDepthMode)

public:
    enum class State
    {
        Stopped, Playing, Paused
    };

    enum class DepthMode
    {
        Base, Normalized, Histogram, Colored
    };


    OniFrameSource(QObject* parent = nullptr);
    ~OniFrameSource();

    int colorFrameWidth() const;
    int colorFrameHeight() const;
    int depthFrameWidth() const;
    int depthFrameHeight() const;
    State state() const;
    QVideoFrame::PixelFormat pixelFormat() const;

    void loadOniFile(const QString& path);

public slots:
    void pause();
    void play();
    void setDepthMode(DepthMode depthMode);
    void setPosition(int position);

signals:
    void stateChanged(OniFrameSource::State newState);

    void durationChanged(int duration);
    void positionChanged(int position);

    void newColorFrame(const QVideoFrame& frame);
    void newDepthFrame(const QVideoFrame& frame);

private slots:
    void processFrame();
    int processColorFrame();
    //void processDepthFrame();
    void processDepthFrameBase();
    void processDepthFrameNormalized();
    void processDepthFrameHistogram();
    void processDepthFrameColored();

private:
    std::pair<quint16, quint16> calculateMinMaxDepthValues(const quint16* depthPixels);
    void calculateHistogram(const quint16* depthPixels);

private:
    openni::Device              m_device;
    openni::PlaybackControl*    m_playbackControl;
    openni::VideoStream         m_colorStream;
    openni::VideoStream         m_depthStream;

    std::unique_ptr<quint8[]>   m_depthFrameBuffer;
    std::unique_ptr<float[]>    m_depthHistogram;

    int                         m_colorFrameWidth;
    int                         m_colorFrameHeight;
    int                         m_depthFrameWidth;
    int                         m_depthFrameHeight;
    State                       m_state;
    QVideoFrame::PixelFormat    m_pixelFormat;
    DepthMode                   m_depthMode;

    int                         m_numberOfFrames;
    int                         m_currentFrame;
    int                         m_framerate;
    QTimer                      m_timer;
};
