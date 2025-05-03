#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QEvent>
#include <QPropertyAnimation>
#include "../../../Model/System/timeLogic.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QMouseEvent> 

class ClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockWidget(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool containsPoint(const QPoint& pos) const;
    bool m_hovered;  // Track hover state manually
    QPoint m_hiddenPos;  // Position when hidden (only 50px visible)
    QPoint m_shownPos;   // Position when fully shown

private:
    QLabel* clockLabel;
    QTimer* updateTimer;
    QPropertyAnimation* slideAnimation;

    void updateTime();
    void slideIn();
    void slideOut();
};

#endif // CLOCK_H
