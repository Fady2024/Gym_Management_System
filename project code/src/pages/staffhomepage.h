#ifndef STAFFHOMEPAGE_H
#define STAFFHOMEPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QObject>
#include "../Language/LanguageSelector.h"
class StaffHomePage : public QWidget
{
    Q_OBJECT

public:
    explicit StaffHomePage(QWidget* parent = nullptr);

signals:
    void navigateBack();

private:
    void setupUI();
};

#endif // STAFFHOMEPAGE_H