#include "Clock.h"

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    setFixedSize(500, 60);
    this->setStyleSheet(clockStyle);

    // Left buttons
    QVBoxLayout* leftButtonsLayout = new QVBoxLayout(this);
    leftButton1 = new QPushButton("ǁǁ");
    leftButton2 = new QPushButton("⏴");
    leftButton1->setFixedWidth(30);
    leftButton2->setFixedWidth(30);
    leftButton1->setStyleSheet(clockButtonStyle);
    leftButton2->setStyleSheet(clockButtonStyle);
    leftButton1->setCursor(Qt::PointingHandCursor);
    leftButton2->setCursor(Qt::PointingHandCursor);
    leftButtonsLayout->addWidget(leftButton1);
    leftButtonsLayout->addWidget(leftButton2);


    clockLabel = new QLabel(this);
    clockLabel->setAlignment(Qt::AlignCenter);
    clockLabel->setStyleSheet(clockLabelStyle);

    // Right buttons
    QVBoxLayout* rightButtonsLayout = new QVBoxLayout(this);
    rightButton1 = new QPushButton("»");
    rightButton2 = new QPushButton("⏵");
    rightButton1->setFixedWidth(30);
    rightButton2->setFixedWidth(30);
    rightButton1->setStyleSheet(clockButtonStyle);
    rightButton2->setStyleSheet(clockButtonStyle);
    rightButton1->setCursor(Qt::PointingHandCursor);
    rightButton2->setCursor(Qt::PointingHandCursor);
    rightButtonsLayout->addWidget(rightButton1);
    rightButtonsLayout->addWidget(rightButton2);


    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ClockWidget::updateTime);
    updateTimer->start(100);

    mainLayout->addLayout(leftButtonsLayout);
    mainLayout->addWidget(clockLabel);
    mainLayout->addLayout(rightButtonsLayout);


    // Connect buttons to slots
    connect(leftButton1, &QPushButton::clicked, this, [=]() {
        timeLogicInstance.pauseTime();
    });
    connect(leftButton2, &QPushButton::clicked, this, [=]() {
        timeLogicInstance.setMultiplier(timeLogicInstance.getMultiplier() - 1);
    });
    connect(rightButton1, &QPushButton::clicked, this, [=]() {
        timeLogicInstance.incrementDays(1);
    });
    connect(rightButton2, &QPushButton::clicked, this, [=]() {
        timeLogicInstance.setMultiplier(timeLogicInstance.getMultiplier() + 1);
    });


}
void ClockWidget::updateTime()
{
        clockLabel->setText(
            timeLogicInstance.getFormattedTime() +
            "   x" +
            QString::number(timeLogicInstance.getMultiplier(), 'f', 1)
        );
}

