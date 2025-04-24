#ifndef LEFTSIDEBAR_H
#define LEFTSIDEBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>
#include <QMap>
#include <QSpacerItem>

class LeftSidebar : public QWidget
{
    Q_OBJECT

public:
    explicit LeftSidebar(QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void addButton(const QString& iconPath, const QString& tooltip, const QString& pageId);
    void addSpacer(int height = 20);
    void addStretch(int stretch = 1);
    void setActiveButton(const QString& pageId);
    void clearButtons();
    void setFixedButtonSize(int size);
    void setButtonSpacing(int spacing);
    void setContentsMargins(int left, int top, int right, int bottom);

signals:
    void pageChanged(const QString& pageId);

private:
    void setupUI();
    QPushButton* createButton(const QString& iconPath, const QString& tooltip, const QString& pageId);
    void updateButtonStyles();

    QVBoxLayout* mainLayout;
    QButtonGroup* buttonGroup;
    QMap<QString, QPushButton*> buttons;
    bool isDarkTheme;
    QString activePageId;
    int buttonSize;
    int buttonSpacing;
};

#endif // LEFTSIDEBAR_H 