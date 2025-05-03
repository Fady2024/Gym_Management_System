#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

class NotificationWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit NotificationWidget(QWidget* parent = nullptr);
    void showNotification(const QString& title, const QString& message, int duration = 3000);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* titleLabel;
    QLabel* messageLabel;
    QPropertyAnimation* slideAnimation;
    QPropertyAnimation* fadeAnimation;
    QTimer* timer;
    qreal m_opacity;

    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    void setupUI();
    void initializeAnimations();
    void startAnimations();
};

#endif // NOTIFICATIONS_H