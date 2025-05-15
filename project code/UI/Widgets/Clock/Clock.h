#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include "../../../Model/System/timeLogic.h"
#include "./Stylesheets/System/clockStyle.h"
#include <QVBoxLayout>
class ClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockWidget(QWidget* parent = nullptr);

protected:
private:
    QLabel* clockLabel;
    QTimer* updateTimer;
    QPushButton* leftButton1;
    QPushButton* leftButton2;
    QPushButton* rightButton1;
    QPushButton* rightButton2;
    void updateTime();

};

#endif // CLOCK_H
