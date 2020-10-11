#include "OniPlayer.hpp"

#include <QtWidgets>
#include <QtMultimediaWidgets/qvideowidget.h>


OniPlayer::OniPlayer(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    connect(&m_frameSource, &OniFrameSource::durationChanged, this, &OniPlayer::durationChanged);
    connect(&m_frameSource, &OniFrameSource::positionChanged, this, &OniPlayer::positionChanged);
    connect(&m_frameSource, &OniFrameSource::stateChanged, this, &OniPlayer::mediaStateChanged);

    auto colorVideoWidget = new QVideoWidget;
    m_colorFrameProvider.setVideoSurface(colorVideoWidget->videoSurface());
    connect(&m_frameSource, &OniFrameSource::newColorFrame, &m_colorFrameProvider, &OniFrameProvider::onNewVideoContentReceived);
    
    auto depthVideoWidget = new QVideoWidget;
    m_depthFrameProvider.setVideoSurface(depthVideoWidget->videoSurface());
    connect(&m_frameSource, &OniFrameSource::newDepthFrame, &m_depthFrameProvider, &OniFrameProvider::onNewVideoContentReceived);

    auto openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &OniPlayer::openFile);

    m_playButton = new QPushButton;
    m_playButton->setEnabled(false);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QAbstractButton::clicked, this, &OniPlayer::play);

    m_frameBackButton = new QPushButton;
    m_frameBackButton->setEnabled(false);
    m_frameBackButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    connect(m_frameBackButton, &QAbstractButton::clicked, this, &OniPlayer::frameBack);

    m_frameForwardButton = new QPushButton;
    m_frameForwardButton->setEnabled(false);
    m_frameForwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    connect(m_frameForwardButton, &QAbstractButton::clicked, this, &OniPlayer::frameForward);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);
    connect(m_positionSlider, &QAbstractSlider::sliderMoved, this, &OniPlayer::setPosition);

    m_errorLabel = new QLabel;
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto framesLayout = new QHBoxLayout;
    framesLayout->addWidget(colorVideoWidget);
    framesLayout->addWidget(depthVideoWidget);

    auto controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_frameBackButton);
    controlLayout->addWidget(m_frameForwardButton);

    auto layout = new QVBoxLayout;
    layout->addLayout(framesLayout);
    layout->addWidget(m_positionSlider);
    layout->addLayout(controlLayout);
    layout->addWidget(m_errorLabel);

    setLayout(layout);
}


void OniPlayer::SetUrl(const QString& url)
{
    m_errorLabel->setText(QString());
    //setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_frameSource.loadOniFile(url);
    m_colorFrameProvider.setFormat(m_frameSource.width(), m_frameSource.height(), m_frameSource.pixelFormat());
    m_depthFrameProvider.setFormat(m_frameSource.width(), m_frameSource.height(), m_frameSource.pixelFormat());

    m_playButton->setEnabled(true);
    m_frameBackButton->setEnabled(true);
    m_frameForwardButton->setEnabled(true);
}


void OniPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open .oni file"));
    // TODO: .oni format?
    /*if (const auto supportedMimeTypes = m_mediaPlayer->supportedMimeTypes(); supportedMimeTypes.isEmpty() == false) {
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }*/
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
        SetUrl(fileDialog.selectedFiles().constFirst());
    }
}

void OniPlayer::play()
{
    switch (m_frameSource.state()) {
        case OniFrameSource::State::Playing:
            m_frameBackButton->setEnabled(true);
            m_frameForwardButton->setEnabled(true);
            m_frameSource.pause();
            break;
        default:
            m_frameBackButton->setEnabled(false);
            m_frameForwardButton->setEnabled(false);
            m_frameSource.play();
            break;
    }
}

void OniPlayer::frameBack()
{
    if (m_frameSource.state() == OniFrameSource::State::Paused) {
        m_frameSource.setPosition(m_positionSlider->value() - 1);
    }
}

void OniPlayer::frameForward()
{
    if (m_frameSource.state() == OniFrameSource::State::Paused) {
        m_frameSource.setPosition(m_positionSlider->value() + 1);
    }
}


void OniPlayer::durationChanged(int duration)
{
    m_positionSlider->setRange(0, duration);
}

void OniPlayer::mediaStateChanged(OniFrameSource::State state)
{
    switch (state) {
        case OniFrameSource::State::Playing:
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        default:
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
    }
}

void OniPlayer::positionChanged(int position)
{
    m_positionSlider->setValue(position);
}

void OniPlayer::setPosition(int position)
{
    m_frameSource.setPosition(position);
}
