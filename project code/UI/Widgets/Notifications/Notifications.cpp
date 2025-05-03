#include "Notifications.h"

NotificationWidget::NotificationWidget(QWidget* parent)
    : QWidget(parent)
    , m_opacity(0.0)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setupUI();
    initializeAnimations();
}

void NotificationWidget::setupUI()
{
    // Set fixed size for the notification
    setFixedSize(300, 100);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(15, 10, 15, 10);

    // Create title label
    titleLabel = new QLabel(this);
    titleLabel->setStyleSheet(notificationLabelStyle);
    
    // Create message label
    messageLabel = new QLabel(this);
    messageLabel->setStyleSheet(messageLabelStyle);
    messageLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(messageLabel);

    // Initialize timer for auto-hide
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [this]() {
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);
        fadeAnimation->start();
    });
}

void NotificationWidget::initializeAnimations()
{
    // Slide animation
    slideAnimation = new QPropertyAnimation(this, "pos", this);
    slideAnimation->setDuration(300);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Fade animation
    fadeAnimation = new QPropertyAnimation(this, "opacity", this);
    fadeAnimation->setDuration(300);
    fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(fadeAnimation, &QPropertyAnimation::finished, [this]() {
        if (fadeAnimation->endValue().toReal() == 0.0) {
            hide();
        }
    });
}

void NotificationWidget::showNotification(const QString& title, const QString& message, std::function<void()> onClick,NotificationType type , int duration)
{
    m_clickCallback = onClick; //reference to the function call we want to execute
    m_isClickable = (onClick != nullptr);
    m_type = type;
    setCursor(m_isClickable? Qt::PointingHandCursor : Qt::ArrowCursor); //sets hand cursor if its clickable
    titleLabel->setText(title);
    messageLabel->setText(message);

    // Calculate position (right-bottom corner)
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    // Position the widget off-screen initially
    QPoint startPos(screenGeometry.width(), screenGeometry.height() - height() - 20);
    QPoint endPos(screenGeometry.width() - width() - 20, screenGeometry.height() - height() - 20);
    
    move(startPos);
    show();

    // Set up slide animation
    slideAnimation->setStartValue(startPos);
    slideAnimation->setEndValue(endPos);

    // Set up fade animation
    fadeAnimation->setStartValue(0.0);
    fadeAnimation->setEndValue(1.0);

    startAnimations();
    timer->start(duration);
}
void NotificationWidget::startAnimations()
{
    slideAnimation->start();
    fadeAnimation->start();
}

void NotificationWidget::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    update();
}

void NotificationWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Enable opacity
    painter.setOpacity(m_opacity);

    // Create gradient background
    QLinearGradient gradient(0, 0, width(), height());

    switch (m_type) {
        case NotificationType::Success:
            gradient.setColorAt(0, QColor(successLight));    // Discord-like blue
            gradient.setColorAt(1, QColor(successDark));     // Slightly darker blue
            break;
        case NotificationType::Error:
            gradient.setColorAt(0, QColor(errorLight));    // Discord-like blue
            gradient.setColorAt(1, QColor(errorDark));     // Slightly darker blue
            break;
        case NotificationType::Info:
        default:
            gradient.setColorAt(0, QColor(infoLight));    // Discord-like blue
            gradient.setColorAt(1, QColor(infoDark));     // Slightly darker blue
            break;
    }


    // Draw rounded rectangle with gradient
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradient);
    painter.drawRoundedRect(rect(), 10, 10);
}
void NotificationWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isClickable && m_clickCallback) {
        m_clickCallback(); //execute function in the lambda showNotification call
    }
}