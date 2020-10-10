#pragma once

#include <QtMultimedia/qabstractvideosurface.h>
#include <QtMultimedia/qvideosurfaceformat.h>
#include <QtMultimedia/qvideoframe.h>


class OniFrameProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractVideoSurface* videoSurface READ videoSurface WRITE setVideoSurface)

public:
    QAbstractVideoSurface* videoSurface() const;

    void setVideoSurface(QAbstractVideoSurface* surface);
    void setFormat(int width, int heigth, QVideoFrame::PixelFormat format);

public slots:
    void onNewVideoContentReceived(const QVideoFrame& frame);

private:
    QAbstractVideoSurface* m_surface = nullptr;
    QVideoSurfaceFormat m_format;
};


//class OniSurface : public QAbstractVideoSurface
//{
//    Q_OBJECT
//
//public:
//    OniSurface(QWidget* widget, QObject* parent = nullptr);
//
//    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
//    bool isFormatSupported(const QVideoSurfaceFormat& format, QVideoSurfaceFormat* similar) const;
//
//    bool start(const QVideoSurfaceFormat& format);
//    void stop();
//
//    bool present(const QVideoFrame& frame);
//
//    QRect videoRect() const { return targetRect; }
//    void updateVideoRect();
//
//    void paint(QPainter* painter);
//
//private:
//    QWidget* widget;
//    QImage::Format imageFormat;
//    QRect targetRect;
//    QSize imageSize;
//    QRect sourceRect;
//    QVideoFrame currentFrame;
//};