#pragma once

#include <QtWidgets/qwidget.h>
#include <QtMultimedia/qabstractvideosurface.h>


class OniSurface : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    OniSurface(QWidget* widget, QObject* parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool isFormatSupported(const QVideoSurfaceFormat& format, QVideoSurfaceFormat* similar) const;

    bool start(const QVideoSurfaceFormat& format);
    void stop();

    bool present(const QVideoFrame& frame);

    QRect videoRect() const { return targetRect; }
    void updateVideoRect();

    void paint(QPainter* painter);

private:
    QWidget* widget;
    QImage::Format imageFormat;
    QRect targetRect;
    QSize imageSize;
    QRect sourceRect;
    QVideoFrame currentFrame;
};
