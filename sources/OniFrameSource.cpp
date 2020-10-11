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
        /*case openni::PixelFormat::PIXEL_FORMAT_GRAY8:
            return QVideoFrame::PixelFormat::Format_Y8;
        case openni::PixelFormat::PIXEL_FORMAT_DEPTH_1_MM:
            return QVideoFrame::PixelFormat::Format_Y16;*/
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
    , m_depthFrameBuffer(nullptr)
{
    openniCheckError(openni::OpenNI::initialize());

    connect(&m_timer, &QTimer::timeout, this, &OniFrameSource::processFrame);
}

OniFrameSource::~OniFrameSource()
{
    if (m_depthFrameBuffer != nullptr) {
        delete[] m_depthFrameBuffer;
    }

    m_colorStream.destroy();
    m_depthStream.destroy();
    m_device.close();

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

QVideoFrame::PixelFormat OniFrameSource::colorPixelFormat() const
{
    return m_colorPixelFormat;
}


void OniFrameSource::loadOniFile(const QString& url)
{
    if (m_depthFrameBuffer != nullptr) {
        delete[] m_depthFrameBuffer;
    }

    m_colorStream.destroy();
    m_depthStream.destroy();
    m_device.close();

    openniCheckError(m_device.open(url.toLocal8Bit().data()));
    openniCheckError(m_colorStream.create(m_device, openni::SensorType::SENSOR_COLOR));
    openniCheckError(m_depthStream.create(m_device, openni::SensorType::SENSOR_DEPTH));

    m_playbackControl = m_device.getPlaybackControl();
    m_playbackControl->setRepeatEnabled(false);
    m_playbackControl->setSpeed(-1.0f);
    m_numberOfFrames = m_playbackControl->getNumberOfFrames(m_colorStream);
    m_currentFrame = -1;

    const auto colorMode = m_colorStream.getVideoMode();
    m_width = colorMode.getResolutionX();
    m_height = colorMode.getResolutionY();
    m_colorPixelFormat = oniToQtPixelFormat(colorMode.getPixelFormat());
    m_framerate = colorMode.getFps();

    m_depthFrameBuffer = new quint8[m_height * m_height * 3];

    /*const auto depthMode = m_depthStream.getVideoMode();
    auto width = depthMode.getResolutionX();
    auto height = depthMode.getResolutionY();
    m_depthPixelFormat = oniToQtPixelFormat(depthMode.getPixelFormat());
    auto framerate = depthMode.getFps();

    printf("DEPTH - width: %d, height: %d, format: %d, framerate: %d\n", width, height, m_depthPixelFormat, framerate);*/

    openniCheckError(m_colorStream.start());
    openniCheckError(m_depthStream.start());

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
        openni::VideoFrameRef colorFrameRef;
        openniCheckError(m_colorStream.readFrame(&colorFrameRef));

        //QSize qsize(frameRef.getWidth(), frameRef.getHeight());
        //auto format = QVideoFrame::PixelFormat::Format_RGB24;
        auto colorBytesPerLine = colorFrameRef.getWidth() * 3;

        QImage colorImage((uchar*)colorFrameRef.getData(), colorFrameRef.getWidth(), colorFrameRef.getHeight(),
                          colorBytesPerLine, QImage::Format::Format_RGB888);
        QVideoFrame colorFrame(colorImage);

        //QVideoFrame frame(frameRef.getDataSize(), qsize, stride, static_cast<QVideoFrame::PixelFormat>(m_pixelFormat));
        //frame.map(QAbstractVideoBuffer::MapMode::WriteOnly);
        //frame.unmap();

        // DEPTH FRAME
        //processDepthFrame1();
        //processDepthFrame2();
        processDepthFrame3();

        emit newColorFrame(colorFrame);
        //emit newDepthFrame(depthFrame);
        emit positionChanged(colorFrameRef.getFrameIndex());

        m_currentFrame = colorFrameRef.getFrameIndex();
        //printf("FRAME: %d, My Frame %d\n", colorFrameRef.getFrameIndex(), m_currentFrame);
    }
    else {
        emit stateChanged(State::Stopped);
    }
}

void OniFrameSource::processDepthFrame1()
{
    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthBytesPerLine = depthFrameRef.getWidth() * 2;

    QImage depthImage((uchar*)depthFrameRef.getData(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      depthBytesPerLine, QImage::Format::Format_Grayscale16);
    QVideoFrame depthFrame(depthImage.convertToFormat(QImage::Format::Format_RGB888));

    emit newDepthFrame(depthFrame);
}

void OniFrameSource::processDepthFrame2()
{
    const auto depthWidth = m_width;
    const auto bufferWidth = m_width * 3;

    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());

    quint16 maxDepthValue = 0;

    // NOTE: Не учитывается 'stride'
    for (int i = 0; i < m_height; ++i) {
        const auto depthRow = i * depthWidth;

        for (int j = 0; j < m_width; ++j) {
            if (maxDepthValue < depthPixels[depthRow + j]) {
                maxDepthValue = depthPixels[depthRow + j];
            }
        }
    }

    for (int i = 0; i < m_height; ++i) {
        const auto depthRow = i * depthWidth;
        const auto bufferRow = i * bufferWidth;

        for (int j = 0; j < m_width; ++j) {
            const auto bufferColumn = bufferRow + j * 3;

            if (depthPixels[depthRow + j] != 0) {
                const quint8 depthValue = float(depthPixels[depthRow + j]) / maxDepthValue * 255;

                m_depthFrameBuffer[bufferColumn] = 255 - depthValue;
                m_depthFrameBuffer[bufferColumn + 1] = 255 - depthValue;
                m_depthFrameBuffer[bufferColumn + 2] = 255 - depthValue;
            }
            else {
                m_depthFrameBuffer[bufferColumn] = 0;
                m_depthFrameBuffer[bufferColumn + 1] = 0;
                m_depthFrameBuffer[bufferColumn + 2] = 0;
            }
        }
    }

    QImage depthImage(m_depthFrameBuffer, depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      bufferWidth, QImage::Format::Format_RGB888);
    QVideoFrame depthFrame(depthImage);

    emit newDepthFrame(depthFrame);
}

void OniFrameSource::processDepthFrame3()
{
    const auto depthWidth = m_width;
    const auto bufferWidth = m_width * 3;

    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());

    int depthHistogram[65536] = {};
    int numberOfDepthValues = 0;

    for (int i = 0; i < m_height; ++i) {
        const auto depthRow = i * depthWidth;

        for (int j = 0; j < m_width; ++j) {
            if (depthPixels[depthRow + j] != 0) {
                ++depthHistogram[depthPixels[depthRow + j]];
                ++numberOfDepthValues;
            }
        }
    }

    for (int i = 1; i < 65536; ++i) {
        depthHistogram[i] += depthHistogram[i - 1];
    }

    for (int i = 0; i < m_height; ++i) {
        const auto depthRow = i * depthWidth;
        const auto bufferRow = i * bufferWidth;

        for (int j = 0; j < m_width; ++j) {
            const auto bufferColumn = bufferRow + j * 3;

            if (depthPixels[depthRow + j] != 0) {
                const quint8 depthValue = float(depthHistogram[depthPixels[depthRow + j]]) / numberOfDepthValues * 255;

                m_depthFrameBuffer[bufferColumn] = 255 - depthValue;
                m_depthFrameBuffer[bufferColumn + 1] = 255 - depthValue;
                m_depthFrameBuffer[bufferColumn + 2] = 255 - depthValue;
            } else {
                m_depthFrameBuffer[bufferColumn] = 0;
                m_depthFrameBuffer[bufferColumn + 1] = 0;
                m_depthFrameBuffer[bufferColumn + 2] = 0;
            }
        }
    }

    QImage depthImage(m_depthFrameBuffer, depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      bufferWidth, QImage::Format::Format_RGB888);
    QVideoFrame depthFrame(depthImage);

    emit newDepthFrame(depthFrame);
}
