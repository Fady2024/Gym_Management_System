#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H
#pragma once

#include <QObject>
#include <QList>
#include <QQueue>
#include "Notifications.h"
#include <QScreen>
#include <QApplication>

class NotificationManager : public QObject {
    Q_OBJECT

public:
    static NotificationManager& instance(); // Singleton

    void showNotification(const QString &title,
                          const QString &message,
                          std::function<void()> onClick = nullptr,
                          NotificationType type = NotificationType::Info,
                          int duration = 3000);

private:
    explicit NotificationManager(QWidget* parent = nullptr);
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;

    // Internal
    void cleanupNotification(NotificationWidget* notif);
    void repositionNotifications();

    QWidget* m_parent = nullptr;
    QList<NotificationWidget*> m_activeNotifications;
};
#endif