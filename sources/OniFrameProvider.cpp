#include "OniFrameProvider.hpp"


QAbstractVideoSurface* OniFrameProvider::videoSurface() const
{
    return m_surface;
}

void OniFrameProvider::setVideoSurface(QAbstractVideoSurface* surface)
{
    if (m_surface && m_surface != surface && m_surface->isActive()) {
        m_surface->stop();
    }

    m_surface = surface;

    if (m_surface && m_format.isValid()) {
        m_format = m_surface->nearestFormat(m_format);
        m_surface->start(m_format);

    }
}

void OniFrameProvider::setFormat(int width, int heigth, QVideoFrame::PixelFormat format)
{
    m_format = QVideoSurfaceFormat(QSize(width, heigth), format);

    if (m_surface) {
        if (m_surface->isActive()) {
            m_surface->stop();
        }
        m_format = m_surface->nearestFormat(m_format);
        m_surface->start(m_format);
    }
}

void OniFrameProvider::onNewVideoContentReceived(const QVideoFrame& frame)
{
    if (m_surface) {
        m_surface->present(frame);
    }
}
