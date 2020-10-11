#pragma once

#include <QtWidgets/qwidget.h>
#include <QtMultimedia/qmediaplayer.h>


#include "OniFrameSource.hpp"
#include "onistream.hpp"


QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSlider;
class QLabel;
class QUrl;
QT_END_NAMESPACE


class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    VideoPlayer(QWidget* parent = nullptr);
    ~VideoPlayer() = default;

    void SetUrl(const QString& url);

public slots:
    void openFile();
    void play();
    void frameBack();
    void frameForward();

private slots:
    void durationChanged(int duration);
    void mediaStateChanged(OniFrameSource::State state);
    void positionChanged(int position);
    void setPosition(qint32 position);
    void handleError();

private:
    OniFrameSource      m_frameSource;
    OniFrameProvider    m_colorFrameProvider;
    OniFrameProvider    m_depthFrameProvider;

    //QMediaPlayer*       m_mediaPlayer;
    QAbstractButton*    m_playButton;
    QAbstractButton*    m_frameBackButton;
    QAbstractButton*    m_frameForwardButton;
    QSlider*            m_positionSlider;
    QLabel*             m_errorLabel;
};
