#include "NotificationManager.h"

NotificationManager& NotificationManager::instance()
{
    static NotificationManager instance;
    return instance;
}

NotificationManager::NotificationManager(QWidget* parent)
    : QObject(parent), m_parent(parent) {}

void NotificationManager::showNotification(const QString& title,
                                           const QString& message,
                                           std::function<void()> onClick,
                                           NotificationType type,
                                           int duration
                                           )
{
    auto* notif = new NotificationWidget(m_parent);
    connect(notif, &NotificationWidget::destroyed, this, [this, notif]() {
        cleanupNotification(notif);
    });

    notif->showNotification(title, message, onClick, type,duration);
    m_activeNotifications.append(notif);
    repositionNotifications();
}

void NotificationManager::cleanupNotification(NotificationWidget* notif)
{
    m_activeNotifications.removeAll(notif);
    repositionNotifications();
}

void NotificationManager::repositionNotifications()
{
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();

    const int spacing = 10;
    int bottomOffset = 20;

    for (int i = m_activeNotifications.size() - 1; i >= 0; --i) {
        NotificationWidget* notif = m_activeNotifications[i];
        QPoint pos(screenGeometry.right() - notif->width() - 20,
                   screenGeometry.bottom() - notif->height() - bottomOffset);
        notif->move(pos);
        bottomOffset += notif->height() + spacing;
    }
}

