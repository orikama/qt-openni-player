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
    void newOniFrameReceived(const QVideoFrame& frame);

private:
    QAbstractVideoSurface*  m_surface = nullptr;
    QVideoSurfaceFormat     m_format;
};
