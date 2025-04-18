#ifndef ADDMEMBER_H
#define ADDMEMBER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QObject>
class AddMember : public QWidget
{
    Q_OBJECT

public:
    explicit AddMember(QWidget* parent = nullptr);

signals:
    void navigateBack();

private:
    void setupUI();
};

#endif // ADDMEMBER_H