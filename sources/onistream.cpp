#include "onistream.hpp"

#include <QtMultimedia/qvideosurfaceformat.h>


OniSurface::OniSurface(QWidget* widget, QObject* parent /*= nullptr*/)
    :QAbstractVideoSurface(parent)
{}


//VideoWidgetSurface::VideoWidgetSurface(QWidget* widget, QObject* parent)
//    : QAbstractVideoSurface(parent)
//    , widget(widget)
//    , imageFormat(QImage::Format_Invalid)
//{}

QList<QVideoFrame::PixelFormat>
OniSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType /*= QAbstractVideoBuffer::NoHandle*/) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle) {
        return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
    } else {
        return QList<QVideoFrame::PixelFormat>();
    }
}

bool OniSurface::isFormatSupported(const QVideoSurfaceFormat& format, QVideoSurfaceFormat* similar) const
{
    Q_UNUSED(similar);

    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    return imageFormat != QImage::Format_Invalid
        && !size.isEmpty()
        && format.handleType() == QAbstractVideoBuffer::NoHandle;
}

bool OniSurface::start(const QVideoSurfaceFormat& format)
{
    const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    if (imageFormat != QImage::Format_Invalid && !size.isEmpty()) {
        this->imageFormat = imageFormat;
        imageSize = size;
        sourceRect = format.viewport();

        QAbstractVideoSurface::start(format);

        widget->updateGeometry();
        updateVideoRect();

        return true;
    }
    else {
        return false;
    }
}

void OniSurface::stop()
{
    currentFrame = QVideoFrame();
    targetRect = QRect();

    QAbstractVideoSurface::stop();

    widget->update();
}

bool OniSurface::present(const QVideoFrame& frame)
{
    if (surfaceFormat().pixelFormat() != frame.pixelFormat() || surfaceFormat().frameSize() != frame.size()) {
        setError(IncorrectFormatError);
        stop();

        return false;
    }
    else {
        currentFrame = frame;

        widget->repaint(targetRect);

        return true;
    }
}

void OniSurface::updateVideoRect()
{
    QSize size = surfaceFormat().sizeHint();
    size.scale(widget->size().boundedTo(size), Qt::KeepAspectRatio);

    targetRect = QRect(QPoint(0, 0), size);
    targetRect.moveCenter(widget->rect().center());
}

void OniSurface::paint(QPainter* painter)
{
    if (currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {
        const QTransform oldTransform = painter->transform();

        if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
            painter->scale(1, -1);
            painter->translate(0, -widget->height());
        }

        QImage image(currentFrame.bits(),
                     currentFrame.width(),
                     currentFrame.height(),
                     currentFrame.bytesPerLine(),
                     imageFormat);

        painter->drawImage(targetRect, image, sourceRect);

        painter->setTransform(oldTransform);

        currentFrame.unmap();
    }
}
