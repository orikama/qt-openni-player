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
    connect(&m_frameSource, &OniFrameSource::newColorFrame, &m_colorFrameProvider, &OniFrameProvider::newOniFrameReceived);
    
    auto depthVideoWidget = new QVideoWidget;
    m_depthFrameProvider.setVideoSurface(depthVideoWidget->videoSurface());
    connect(&m_frameSource, &OniFrameSource::newDepthFrame, &m_depthFrameProvider, &OniFrameProvider::newOniFrameReceived);

    auto openButton = new QPushButton("Open...");
    connect(openButton, &QAbstractButton::clicked, this, &OniPlayer::openFile);

    m_depthModeComboBox = new QComboBox;
    m_depthModeComboBox->addItem("Base");
    m_depthModeComboBox->addItem("Normalized");
    m_depthModeComboBox->addItem("Histogram");
    m_depthModeComboBox->addItem("Colored");
    connect(m_depthModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OniPlayer::depthModeChanged);

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

    auto framesLayout = new QHBoxLayout;
    framesLayout->addWidget(colorVideoWidget);
    framesLayout->addWidget(depthVideoWidget);

    auto controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_depthModeComboBox);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_frameBackButton);
    controlLayout->addWidget(m_frameForwardButton);

    auto layout = new QVBoxLayout;
    layout->addLayout(framesLayout);
    layout->addWidget(m_positionSlider);
    layout->addLayout(controlLayout);

    setLayout(layout);
}


void OniPlayer::SetFile(const QString& filePath)
{
    m_frameSource.loadOniFile(filePath);
    m_colorFrameProvider.setFormat(m_frameSource.colorFrameWidth(), m_frameSource.colorFrameHeight(), m_frameSource.pixelFormat());
    m_depthFrameProvider.setFormat(m_frameSource.depthFrameWidth(), m_frameSource.depthFrameHeight(), m_frameSource.pixelFormat());

    m_playButton->setEnabled(true);
    m_frameBackButton->setEnabled(true);
    m_frameForwardButton->setEnabled(true);

    m_positionSlider->setValue(0);
}


void OniPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle("Open .oni file");
    fileDialog.setNameFilter("Oni (*.oni)");
    if (fileDialog.exec() == QDialog::Accepted) {
        SetFile(fileDialog.selectedFiles().constFirst());
    }
}

void OniPlayer::play()
{
    if (m_frameSource.state() == OniFrameSource::State::Playing) {
        m_frameBackButton->setEnabled(true);
        m_frameForwardButton->setEnabled(true);
        m_frameSource.pause();
    }
    else {
        m_frameBackButton->setEnabled(false);
        m_frameForwardButton->setEnabled(false);
        m_frameSource.play();
    }
}

void OniPlayer::frameBack()
{
    if (m_frameSource.state() != OniFrameSource::State::Playing) {
        m_frameSource.setPosition(m_positionSlider->value() - 1);
    }
}

void OniPlayer::frameForward()
{
    if (m_frameSource.state() != OniFrameSource::State::Playing) {
        m_frameSource.setPosition(m_positionSlider->value() + 1);
    }
}


void OniPlayer::durationChanged(int duration)
{
    m_positionSlider->setRange(0, duration);
}

void OniPlayer::depthModeChanged(int index)
{
    m_frameSource.setDepthMode(static_cast<OniFrameSource::DepthMode>(index));
}

void OniPlayer::mediaStateChanged(OniFrameSource::State state)
{
    if (state == OniFrameSource::State::Playing) {
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
    else {
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
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
