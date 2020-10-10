#include "OniFrameSource.hpp"

#include <QtCore/qurl.h>

#include <algorithm>


void openniCheckError(openni::Status status)
{
    if (status != openni::Status::STATUS_OK) {
        printf("ERROR::OpenNI: #%d, %s", status, openni::OpenNI::getExtendedError());
    }
}



QVideoFrame::PixelFormat oniToQtPixelFormat(openni::PixelFormat oniFormat)
{
    switch (oniFormat) {
        case openni::PixelFormat::PIXEL_FORMAT_RGB888:
            return QVideoFrame::PixelFormat::Format_RGB24;
    }
}

QImage::Format oniToQtFormat(openni::PixelFormat oniFormat)
{
    switch (oniFormat) {
        case openni::PixelFormat::PIXEL_FORMAT_RGB888:
            return QImage::Format::Format_RGB888;
    }
}


OniFrameSource::OniFrameSource()
    : m_state(State::Stopped)
{
    openniCheckError(openni::OpenNI::initialize());

    connect(&m_timer, &QTimer::timeout, this, &OniFrameSource::processFrame);
}

OniFrameSource::~OniFrameSource()
{
    m_device.close();
    m_colorStream.destroy();

    openni::OpenNI::shutdown();
}


OniFrameSource::State OniFrameSource::state() const
{
    return m_state;
}

int OniFrameSource::width() const
{
    return m_width;
}

int OniFrameSource::height() const
{
    return m_height;
}

QVideoFrame::PixelFormat OniFrameSource::pixelFormat() const
{
    return m_pixelFormat;
}


void OniFrameSource::loadOniFile(const QString& url)
{
    if (m_colorStream.isValid()) {
        m_colorStream.destroy();
    }
    if (m_device.isValid()) {
        m_device.close();
    }

    openniCheckError(m_device.open(url.toLocal8Bit().data()));
    openniCheckError(m_colorStream.create(m_device, openni::SensorType::SENSOR_COLOR));

    m_playbackControl = m_device.getPlaybackControl();
    m_playbackControl->setRepeatEnabled(false);
    m_playbackControl->setSpeed(-1.0f);
    m_numberOfFrames = m_playbackControl->getNumberOfFrames(m_colorStream);
    m_currentFrame = -1;

    auto mode = m_colorStream.getVideoMode();
    m_width = mode.getResolutionX();
    m_height = mode.getResolutionY();
    m_pixelFormat = oniToQtPixelFormat(mode.getPixelFormat());
    m_framerate = mode.getFps();

    openniCheckError(m_colorStream.start());

    emit durationChanged(m_numberOfFrames);
}

void OniFrameSource::pause()
{
    if (m_state == State::Playing) {
        m_timer.stop();

        m_state = State::Paused;
        emit stateChanged(m_state);
    }
}

void OniFrameSource::play()
{
    if (m_state != State::Playing) {

        m_state = State::Playing;
        emit stateChanged(m_state);

        m_timer.start(1000 / m_framerate);
    }
}

void OniFrameSource::setPosition(int position)
{
    if (m_playbackControl != nullptr && m_playbackControl->isValid()) {
        position = std::clamp(position, 2, m_numberOfFrames);
        if (position != m_currentFrame) {
            m_currentFrame = position;
            m_playbackControl->seek(m_colorStream, position);
            
            processFrame();
        }
    }
}

void OniFrameSource::processFrame()
{
    if (m_currentFrame < m_numberOfFrames - 1) {
        openni::VideoFrameRef frameRef;
        openniCheckError(m_colorStream.readFrame(&frameRef));

        //QSize qsize(frameRef.getWidth(), frameRef.getHeight());
        //auto format = QVideoFrame::PixelFormat::Format_RGB24;
        auto stride = frameRef.getWidth() * 3;

        QImage image((uchar*)frameRef.getData(), frameRef.getWidth(), frameRef.getHeight(),
                     stride, QImage::Format::Format_RGB888);
        QVideoFrame frame(image);

        //QVideoFrame frame(frameRef.getDataSize(), qsize, stride, static_cast<QVideoFrame::PixelFormat>(m_pixelFormat));
        //frame.map(QAbstractVideoBuffer::MapMode::WriteOnly);
        //frame.unmap();

        emit newFrameAvailable(frame);
        emit positionChanged(frameRef.getFrameIndex());
        m_currentFrame = frameRef.getFrameIndex();
        printf("FRAME: %d, My Frame %d\n", frameRef.getFrameIndex(), m_currentFrame);
    }
    else {
        emit stateChanged(State::Stopped);
    }
}
