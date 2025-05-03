
#include "Clock.h"


int hide_offset = 280;
int show_offset = 150;

ClockWidget::ClockWidget(QWidget* parent)
    : QWidget(parent), m_hovered(false)
{
    
    setFixedSize(500, 60);
    setAttribute(Qt::WA_Hover);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setStyleSheet("background-color: rgba(0, 0, 0, 0.7); color: white; border-radius: 10px");
    
    clockLabel = new QLabel(this);
    clockLabel->setAlignment(Qt::AlignCenter);
    clockLabel->setGeometry(0, 0, width(), height());
    clockLabel->setStyleSheet("font-size: 18px;");

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ClockWidget::updateTime);
    updateTimer->start(100);

    slideAnimation = new QPropertyAnimation(this, "pos");
    slideAnimation->setDuration(300);
    connect(slideAnimation, &QPropertyAnimation::valueChanged, this, [this]() {
        if (m_hovered != underMouse()) {
            m_hovered = underMouse();
            if (m_hovered) slideIn();
            else slideOut();
        }
        });

    // Start hidden (only 50px visible)
    m_hiddenPos = QPoint(parent->width() + hide_offset, (parent->height() - height()) / 2);
    m_shownPos = QPoint(parent->width() - show_offset, (parent->height() - height()) / 2);
    move(m_hiddenPos);
}
void ClockWidget::updateTime()
{
        clockLabel->setText(
            timeLogicInstance.getFormattedTime() +
            "   x" +
            QString::number(timeLogicInstance.getMultiplier(), 'f', 1)
        );
}

void ClockWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // Update positions in case parent size changed
    m_hiddenPos = QPoint(parentWidget()->width() + hide_offset, (parentWidget()->height() - height()) / 2);
    m_shownPos = QPoint(parentWidget()->width() - show_offset, (parentWidget()->height() - height()) / 2);
    move(m_hiddenPos);
}
bool ClockWidget::event(QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        if (auto* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
            m_hovered = rect().contains(mouseEvent->pos());
            m_hovered ? slideIn() : slideOut();
        }
    }
    else if (event->type() == QEvent::Enter) {
        m_hovered = true;
        slideIn();
    }
    else if (event->type() == QEvent::Leave) {
        m_hovered = false;
        slideOut();
    }
    return QWidget::event(event);
}

void ClockWidget::slideIn()
{
    if (slideAnimation->state() == QPropertyAnimation::Running)
        slideAnimation->stop();

    slideAnimation->setStartValue(pos());
    slideAnimation->setEndValue(m_shownPos);
    slideAnimation->start();
}

void ClockWidget::slideOut()
{
    if (slideAnimation->state() == QPropertyAnimation::Running)
        slideAnimation->stop();

    slideAnimation->setStartValue(pos());
    slideAnimation->setEndValue(m_hiddenPos);
    slideAnimation->start();
}

void ClockWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Update positions when widget resizes
    m_hiddenPos = QPoint(parentWidget()->width() + hide_offset, (parentWidget()->height() - height()) / 2);
    m_shownPos = QPoint(parentWidget()->width() - show_offset, (parentWidget()->height() - height()) / 2);

    if (m_hovered)
        move(m_shownPos);
    else
        move(m_hiddenPos);
}