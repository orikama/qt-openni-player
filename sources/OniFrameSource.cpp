#include "OniFrameSource.hpp"

#include <algorithm>
//#include <functional>
//#include <tuple>


constexpr auto kMaxDepthValue = 65536;


void openniCheckError(openni::Status status)
{
    if (status != openni::Status::STATUS_OK) {
        printf("ERROR::OpenNI: #%d, %s", status, openni::OpenNI::getExtendedError());
    }
}


//QVideoFrame::PixelFormat oniToQtPixelFormat(openni::PixelFormat oniFormat)
//{
//    switch (oniFormat) {
//        case openni::PixelFormat::PIXEL_FORMAT_RGB888:
//            return QVideoFrame::PixelFormat::Format_RGB24;
//        /*case openni::PixelFormat::PIXEL_FORMAT_GRAY8:
//            return QVideoFrame::PixelFormat::Format_Y8;
//        case openni::PixelFormat::PIXEL_FORMAT_DEPTH_1_MM:
//            return QVideoFrame::PixelFormat::Format_Y16;*/
//    }
//}
//
//QImage::Format oniToQtFormat(openni::PixelFormat oniFormat)
//{
//    switch (oniFormat) {
//        case openni::PixelFormat::PIXEL_FORMAT_RGB888:
//            return QImage::Format::Format_RGB888;
//    }
//}


OniFrameSource::OniFrameSource(QObject* parent /*= nullptr*/)
    : QObject(parent)
    , m_playbackControl(nullptr)
    , m_depthFrameBuffer(nullptr)
    , m_depthHistogram(new float[kMaxDepthValue])
    , m_colorFrameWidth(-1)
    , m_colorFrameHeight(-1)
    , m_depthFrameWidth(-1)
    , m_depthFrameHeight(-1)
    , m_state(State::Stopped)
    , m_pixelFormat()
    , m_depthMode(DepthMode::Base)
    , m_numberOfFrames(-1)
    , m_currentFrame(-1)
    , m_framerate(-1)
{
    openniCheckError(openni::OpenNI::initialize());

    connect(&m_timer, &QTimer::timeout, this, &OniFrameSource::processFrame);
}

OniFrameSource::~OniFrameSource()
{
    m_timer.stop();

    m_colorStream.destroy();
    m_depthStream.destroy();
    m_device.close();

    openni::OpenNI::shutdown();
}


int OniFrameSource::colorFrameWidth() const
{
    return m_colorFrameWidth;
}

int OniFrameSource::colorFrameHeight() const
{
    return m_colorFrameHeight;
}

int OniFrameSource::depthFrameWidth() const
{
    return m_depthFrameWidth;
}

int OniFrameSource::depthFrameHeight() const
{
    return m_depthFrameHeight;
}

OniFrameSource::State OniFrameSource::state() const
{
    return m_state;
}

QVideoFrame::PixelFormat OniFrameSource::pixelFormat() const
{
    return m_pixelFormat;
}


void  OniFrameSource::setDepthMode(DepthMode depthMode)
{
    m_depthMode = depthMode;
}


void OniFrameSource::loadOniFile(const QString& path)
{
    m_timer.stop();

    m_colorStream.destroy();
    m_depthStream.destroy();
    m_device.close();

    m_state = State::Stopped;
    m_currentFrame = -1;

    openniCheckError(m_device.open(path.toLocal8Bit().data()));
    openniCheckError(m_colorStream.create(m_device, openni::SensorType::SENSOR_COLOR));
    openniCheckError(m_depthStream.create(m_device, openni::SensorType::SENSOR_DEPTH));

    m_device.setDepthColorSyncEnabled(true);
    m_playbackControl = m_device.getPlaybackControl();
    m_playbackControl->setRepeatEnabled(false);
    m_playbackControl->setSpeed(-1.0f);
    m_numberOfFrames = m_playbackControl->getNumberOfFrames(m_colorStream);

    const auto colorMode = m_colorStream.getVideoMode();
    m_colorFrameWidth = colorMode.getResolutionX();
    m_colorFrameHeight = colorMode.getResolutionY();
    m_pixelFormat = QVideoFrame::PixelFormat::Format_RGB24; //oniToQtPixelFormat(colorMode.getPixelFormat());
    m_framerate = colorMode.getFps();

    const auto depthMode = m_depthStream.getVideoMode();
    m_depthFrameWidth = depthMode.getResolutionX();
    m_depthFrameHeight = depthMode.getResolutionY();
    m_depthFrameBuffer = std::make_unique<quint8[]>(m_depthFrameHeight * m_depthFrameWidth * 3);

    openniCheckError(m_colorStream.start());
    openniCheckError(m_depthStream.start());

    m_playbackControl->seek(m_colorStream, 100);

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

        if (m_currentFrame == m_numberOfFrames) {
            setPosition(0);
        }

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
    if (m_currentFrame < m_numberOfFrames) {
        m_currentFrame = processColorFrame();

        switch (m_depthMode) {
            case DepthMode::Base:
                processDepthFrameBase();
                break;
            case DepthMode::Normalized:
                processDepthFrameNormalized();
                break;
            case DepthMode::Histogram:
                processDepthFrameHistogram();
                break;
            case DepthMode::Colored:
                processDepthFrameColored();
                break;
        }

        emit positionChanged(m_currentFrame);
    }
    else {
        m_timer.stop();
        m_state = State::Stopped;
        emit stateChanged(State::Stopped);
    }
}

int OniFrameSource::processColorFrame()
{
    openni::VideoFrameRef colorFrameRef;
    openniCheckError(m_colorStream.readFrame(&colorFrameRef));

    auto colorBytesPerLine = colorFrameRef.getWidth() * 3;

    QImage colorImage((uchar*)colorFrameRef.getData(), colorFrameRef.getWidth(), colorFrameRef.getHeight(),
                      colorBytesPerLine, QImage::Format::Format_RGB888);
    QVideoFrame colorFrame(colorImage);

    emit newColorFrame(colorFrame);

    return colorFrameRef.getFrameIndex();
}

void OniFrameSource::processDepthFrameBase()
{
    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthBytesPerLine = depthFrameRef.getWidth() * 2;

    QImage depthImage((uchar*)depthFrameRef.getData(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      depthBytesPerLine, QImage::Format::Format_Grayscale16);
    QVideoFrame depthFrame(depthImage.convertToFormat(QImage::Format::Format_RGB888));

    emit newDepthFrame(depthFrame);
}

// Ожидаемо выдает приемлимый ФПС только в релиз билде
//void OniFrameSource::processDepthFrame()
//{
//    const auto depthWidth = m_width;
//    const auto bufferWidth = m_width * 3;
//
//    openni::VideoFrameRef depthFrameRef;
//    openniCheckError(m_depthStream.readFrame(&depthFrameRef));
//
//    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());
//
//    auto [minDepthValue, maxDepthValue] = calculateMinMaxDepthValues(depthPixels);
//
//    std::function<std::tuple<quint8, quint8, quint8>(quint16 depthValue)> bla_bla;
//
//    switch (m_depthMode) {
//        case DepthMode::Normalized: {
//                auto pair = calculateMinMaxDepthValues(depthPixels);
//
//                bla_bla = [maxDepthValue = pair.second](quint16 depthPixel)
//                {
//                    quint8 depthValue = float(depthPixel) / maxDepthValue * 255;
//                    depthValue = 255 - depthValue;
//
//                    return std::make_tuple(depthValue, depthValue, depthValue);
//                };
//            }
//            break;
//        case DepthMode::Histogram: {
//                calculateHistogram(depthPixels);
//
//                bla_bla = [histogram = m_depthHistogram.get()](quint16 depthPixel)
//                {
//                    quint8 depthValue = histogram[depthPixel] * 255;
//                    depthValue = 255 - depthValue;
//
//                    return std::make_tuple(depthValue, depthValue, depthValue);
//                };
//            }
//            break;
//        case DepthMode::Colored: {
//                auto pair = calculateMinMaxDepthValues(depthPixels);
//                const auto colorFactor = 1024.0f / maxDepthValue;
//
//                bla_bla = [minDepthValue = pair.first, colorFactor](quint16 depthPixel)
//                {
//                    const int color = (depthPixel - minDepthValue) * colorFactor;
//
//                    quint8 r = color > 0 && color < 512 ? std::abs(color - 256) : 255;
//                    quint8 g = color > 128 && color < 640 ? std::abs(color - 384) : 255;
//                    quint8 b = color > 512 && color < 1024 ? std::abs(color - 768) : 255;
//
//                    return std::make_tuple(b, g, r);
//                };
//            }
//            break;
//    }
//
//    for (int i = 0; i < m_height; ++i) {
//        const auto depthRow = i * depthWidth;
//        const auto bufferRow = i * bufferWidth;
//
//        for (int j = 0; j < m_width; ++j) {
//            const auto bufferColumn = bufferRow + j * 3;
//
//            if (depthPixels[depthRow + j] != 0) {
//                const auto [r, g, b] = bla_bla(depthPixels[depthRow + j]);
//                m_depthFrameBuffer[bufferColumn] = r;
//                m_depthFrameBuffer[bufferColumn + 1] = g;
//                m_depthFrameBuffer[bufferColumn + 2] = b;
//            } else {
//                m_depthFrameBuffer[bufferColumn] = 0;
//                m_depthFrameBuffer[bufferColumn + 1] = 0;
//                m_depthFrameBuffer[bufferColumn + 2] = 0;
//            }
//        }
//    }
//
//    QImage depthImage(m_depthFrameBuffer.get(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
//                      bufferWidth, QImage::Format::Format_RGB888);
//    QVideoFrame depthFrame(depthImage);
//
//    emit newDepthFrame(depthFrame);
//}

void OniFrameSource::processDepthFrameNormalized()
{
    const auto depthWidth = m_depthFrameWidth;
    const auto bufferWidth = m_depthFrameWidth * 3;

    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());

    const auto [_, maxDepthValue] = calculateMinMaxDepthValues(depthPixels);

    for (int i = 0; i < m_depthFrameHeight; ++i) {
        const auto depthRow = i * depthWidth;
        const auto bufferRow = i * bufferWidth;

        for (int j = 0; j < m_depthFrameWidth; ++j) {
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

    QImage depthImage(m_depthFrameBuffer.get(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      bufferWidth, QImage::Format::Format_RGB888);
    QVideoFrame depthFrame(depthImage);

    emit newDepthFrame(depthFrame);
}

void OniFrameSource::processDepthFrameHistogram()
{
    const auto depthWidth = m_depthFrameWidth;
    const auto bufferWidth = m_depthFrameWidth * 3;

    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());

    calculateHistogram(depthPixels);

    for (int i = 0; i < m_depthFrameHeight; ++i) {
        const auto depthRow = i * depthWidth;
        const auto bufferRow = i * bufferWidth;

        for (int j = 0; j < m_depthFrameWidth; ++j) {
            const auto bufferColumn = bufferRow + j * 3;

            if (depthPixels[depthRow + j] != 0) {
                const quint8 depthValue = m_depthHistogram[depthPixels[depthRow + j]] * 255;

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

    QImage depthImage(m_depthFrameBuffer.get(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      bufferWidth, QImage::Format::Format_RGB888);
    QVideoFrame depthFrame(depthImage);

    emit newDepthFrame(depthFrame);
}

void OniFrameSource::processDepthFrameColored()
{
    const auto depthWidth = m_depthFrameWidth;
    const auto bufferWidth = m_depthFrameWidth * 3;

    openni::VideoFrameRef depthFrameRef;
    openniCheckError(m_depthStream.readFrame(&depthFrameRef));

    auto depthPixels = static_cast<const quint16*>(depthFrameRef.getData());

    const auto [minDepthValue, maxDepthValue] = calculateMinMaxDepthValues(depthPixels);
    const auto colorFactor = 1024.0f / maxDepthValue;

    for (int i = 0; i < m_depthFrameHeight; ++i) {
        const auto depthRow = i * depthWidth;
        const auto bufferRow = i * bufferWidth;

        for (int j = 0; j < m_depthFrameWidth; ++j) {
            const auto bufferColumn = bufferRow + j * 3;

            if (depthPixels[depthRow + j] != 0) {
                const int color = (depthPixels[depthRow + j] - minDepthValue) * colorFactor;

                m_depthFrameBuffer[bufferColumn] = color > 0 && color < 512 ? std::abs(color - 256) : 255;
                m_depthFrameBuffer[bufferColumn + 1] = color > 128 && color < 640 ? std::abs(color - 384) : 255;
                m_depthFrameBuffer[bufferColumn + 2] = color > 512 && color < 1024 ? std::abs(color - 768) : 255;
            } else {
                m_depthFrameBuffer[bufferColumn] = 0;
                m_depthFrameBuffer[bufferColumn + 1] = 0;
                m_depthFrameBuffer[bufferColumn + 2] = 0;
            }
        }
    }

    QImage depthImage(m_depthFrameBuffer.get(), depthFrameRef.getWidth(), depthFrameRef.getHeight(),
                      bufferWidth, QImage::Format::Format_RGB888);
    QVideoFrame depthFrame(depthImage);

    emit newDepthFrame(depthFrame);
}


std::pair<quint16, quint16> OniFrameSource::calculateMinMaxDepthValues(const quint16* depthPixels)
{
    const auto depthWidth = m_depthFrameWidth;

    quint16 maxDepthValue = m_depthStream.getMinPixelValue();
    quint16 minDepthValue = m_depthStream.getMaxPixelValue();

    for (int i = 0; i < m_depthFrameHeight; ++i) {
        const auto depthRow = i * depthWidth;

        for (int j = 0; j < m_depthFrameWidth; ++j) {
            const auto depthValue = depthPixels[depthRow + j];

            if (maxDepthValue < depthValue) {
                maxDepthValue = depthValue;
            }
            if (depthValue != 0 && minDepthValue > depthValue) {
                minDepthValue = depthValue;
            }
        }
    }

    return { minDepthValue, maxDepthValue };
}

void OniFrameSource::calculateHistogram(const quint16* depthPixels)
{
    const auto depthWidth = m_depthFrameWidth;
    auto numberOfDepthValues = 0;

    std::fill(m_depthHistogram.get(), m_depthHistogram.get() + kMaxDepthValue, 0);

    for (int i = 0; i < m_depthFrameHeight; ++i) {
        const auto depthRow = i * depthWidth;

        for (int j = 0; j < m_depthFrameWidth; ++j) {
            if (depthPixels[depthRow + j] != 0) {
                ++m_depthHistogram[depthPixels[depthRow + j]];
                ++numberOfDepthValues;
            }
        }
    }

    for (int i = 1; i < kMaxDepthValue; ++i) {
        m_depthHistogram[i] += m_depthHistogram[i - 1];
    }

    for (int i = 1; i < kMaxDepthValue; ++i) {
        if (m_depthHistogram[i] != 0) {
            m_depthHistogram[i] /= numberOfDepthValues;
        }
    }
}
