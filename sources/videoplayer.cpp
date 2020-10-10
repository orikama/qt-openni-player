#include "videoplayer.hpp"

#include <QtWidgets>
#include <QtMultimediaWidgets/qvideowidget.h>


VideoPlayer::VideoPlayer(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    auto videoWidget = new QVideoWidget;

    auto openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::OpenFile);

    m_playButton = new QPushButton;
    m_playButton->setEnabled(false);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QAbstractButton::clicked, this, &VideoPlayer::Play);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);
    connect(m_positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::setPosition);

    m_errorLabel = new QLabel;
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_positionSlider);

    auto layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addLayout(controlLayout);
    layout->addWidget(m_errorLabel);

    setLayout(layout);

    m_mediaPlayer->setVideoOutput(videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::handleError);
}


void VideoPlayer::SetUrl(const QUrl& url)
{
    m_errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    m_mediaPlayer->setMedia(url);
    m_playButton->setEnabled(true);
}


void VideoPlayer::OpenFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    if (const auto supportedMimeTypes = m_mediaPlayer->supportedMimeTypes(); supportedMimeTypes.isEmpty() == false) {
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted) {
        SetUrl(fileDialog.selectedUrls().constFirst());
    }
}

void VideoPlayer::Play()
{
    switch (m_mediaPlayer->state()) {
        case QMediaPlayer::PlayingState:
            m_mediaPlayer->pause();
            break;
        default:
            m_mediaPlayer->play();
            break;
    }
}


void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch (state) {
        case QMediaPlayer::PlayingState:
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        default:
            m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
    m_positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(qint32 position)
{
    m_mediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
{
    m_playButton->setEnabled(false);
    const auto errorString = m_mediaPlayer->errorString();

    QString message = "Error: ";
    if (const auto errorString = m_mediaPlayer->errorString(); errorString.isEmpty() == false) {
        message += errorString;
    }
    else {
        message += " #" + QString::number(m_mediaPlayer->error());
    }

    m_errorLabel->setText(message);
}
