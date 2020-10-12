#pragma once

#include <QtWidgets/qwidget.h>
#include <QtMultimedia/qmediaplayer.h>

#include "OniFrameSource.hpp"
#include "OniFrameProvider.hpp"


QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSlider;
class QLabel;
class QUrl;
class QComboBox;
QT_END_NAMESPACE


class OniPlayer : public QWidget
{
    Q_OBJECT

public:
    OniPlayer(QWidget* parent = nullptr);
    ~OniPlayer() = default;

    void SetUrl(const QString& url);

public slots:
    void openFile();
    void play();
    void frameBack();
    void frameForward();

private slots:
    void durationChanged(int duration);
    void depthModeChanged(int index);
    void mediaStateChanged(OniFrameSource::State state);
    void positionChanged(int position);
    void setPosition(int position);

private:
    OniFrameSource      m_frameSource;
    OniFrameProvider    m_colorFrameProvider;
    OniFrameProvider    m_depthFrameProvider;

    QComboBox*          m_depthModeComboBox;
    QAbstractButton*    m_playButton;
    QAbstractButton*    m_frameBackButton;
    QAbstractButton*    m_frameForwardButton;
    QSlider*            m_positionSlider;
    QLabel*             m_errorLabel;
};
