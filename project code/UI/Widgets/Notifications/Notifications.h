#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QWidget>
#include <QPainter>
#include <QVBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QMouseEvent>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>
#include <functional>
#include "Stylesheets/System/notificationStyle.h"

enum class NotificationType {
    Info,
    Success,
    Error
};

class NotificationWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit NotificationWidget(QWidget* parent = nullptr);
    // void showNotification(const QString& title, const QString& message, int duration);
    void showNotification(const QString& title, const QString& message, std::function<void()> onClick = nullptr, NotificationType type = NotificationType::Info, int duration = 3000);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* titleLabel;
    QLabel* messageLabel;
    QPropertyAnimation* slideAnimation;
    QPropertyAnimation* fadeAnimation;
    QTimer* timer;
    qreal m_opacity;
    NotificationType m_type = NotificationType::Info;
    bool m_isClickable = false;
    std::function<void()> m_clickCallback = nullptr;


    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    void setupUI();
    void initializeAnimations();
    void startAnimations();
    void mousePressEvent(QMouseEvent* event) override;
};

#endif // NOTIFICATIONS_H