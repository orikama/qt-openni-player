#include "videoplayer.hpp"

#include <QtWidgets>
#include <QtMultimediaWidgets/qvideowidget.h>


VideoPlayer::VideoPlayer(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    //m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    auto videoWidget = new QVideoWidget;
    m_frameProvider.setVideoSurface(videoWidget->videoSurface());
    connect(&m_frameSource, &OniFrameSource::newFrameAvailable, &m_frameProvider, &OniFrameProvider::onNewVideoContentReceived);
    connect(&m_frameSource, &OniFrameSource::durationChanged, this, &VideoPlayer::durationChanged);
    connect(&m_frameSource, &OniFrameSource::positionChanged, this, &VideoPlayer::positionChanged);
    connect(&m_frameSource, &OniFrameSource::stateChanged, this, &VideoPlayer::mediaStateChanged);

    auto openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);

    m_playButton = new QPushButton;
    m_playButton->setEnabled(false);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QAbstractButton::clicked, this, &VideoPlayer::play);

    m_frameBackButton = new QPushButton;
    m_frameBackButton->setEnabled(false);
    m_frameBackButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    connect(m_frameBackButton, &QAbstractButton::clicked, this, &VideoPlayer::frameBack);

    m_frameForwardButton = new QPushButton;
    m_frameForwardButton->setEnabled(false);
    m_frameForwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    connect(m_frameForwardButton, &QAbstractButton::clicked, this, &VideoPlayer::frameForward);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);
    connect(m_positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::setPosition);

    m_errorLabel = new QLabel;
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_frameBackButton);
    controlLayout->addWidget(m_frameForwardButton);

    auto layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addWidget(m_positionSlider);
    layout->addLayout(controlLayout);
    layout->addWidget(m_errorLabel);

    setLayout(layout);

    /*m_mediaPlayer->setVideoOutput(videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::handleError);*/
}


void VideoPlayer::SetUrl(const QString& url)
{
    m_errorLabel->setText(QString());
    //setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    //m_mediaPlayer->setMedia(url);
    m_frameSource.loadOniFile(url);
    m_frameProvider.setFormat(m_frameSource.width(), m_frameSource.height(), m_frameSource.pixelFormat());

    m_playButton->setEnabled(true);
    m_frameBackButton->setEnabled(true);
    m_frameForwardButton->setEnabled(true);
}


void VideoPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    // TODO: .oni format?
    /*if (const auto supportedMimeTypes = m_mediaPlayer->supportedMimeTypes(); supportedMimeTypes.isEmpty() == false) {
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }*/
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
        SetUrl(fileDialog.selectedFiles().constFirst() /*.selectedUrls().constFirst()*/);
    }
}

void VideoPlayer::play()
{
    switch (m_frameSource.state()) {
        case OniFrameSource::State::Playing:
            m_frameSource.pause();
            break;
        default:
            m_frameSource.play();
            break;
    }
}

void VideoPlayer::frameBack()
{
    if(m_frameSource.state() == OniFrameSource::State::Paused) {
        m_frameSource.setPosition(m_positionSlider->value() - 1);
    }
}

void VideoPlayer::frameForward()
{
    if (m_frameSource.state() == OniFrameSource::State::Paused) {
        m_frameSource.setPosition(m_positionSlider->value() + 1);
    }
}


void VideoPlayer::durationChanged(int duration)
{
    m_positionSlider->setRange(0, duration);
}

void VideoPlayer::mediaStateChanged(OniFrameSource::State state)
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

void VideoPlayer::positionChanged(int position)
{
    m_positionSlider->setValue(position);
}

void VideoPlayer::setPosition(int position)
{
   m_frameSource.setPosition(position);
}

void VideoPlayer::handleError()
{
    /*m_playButton->setEnabled(false);
    const auto errorString = m_mediaPlayer->errorString();

    QString message = "Error: ";
    if (const auto errorString = m_mediaPlayer->errorString(); errorString.isEmpty() == false) {
        message += errorString;
    }
    else {
        message += " #" + QString::number(m_mediaPlayer->error());
    }

    m_errorLabel->setText(message);*/
}
