#ifndef LANGUAGESELECTIONPAGE_H
#define LANGUAGESELECTIONPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "../Util/LanguageSelector.h"

class LanguageSelectionPage : public QWidget
{
    Q_OBJECT

public:
    explicit LanguageSelectionPage(QWidget* parent = nullptr);
    void startAnimation();
    void updateTheme(bool isDark);

signals:
    void languageSelected();

private slots:
    void onContinueClicked();

protected:
    bool event(QEvent* e) override;

private:
    void setupUI();
    void setupAnimations();
    void retranslateUI();

    QWidget* container;
    QLabel* logoLabel;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    LanguageSelector* languageSelector;
    QPushButton* continueButton;
    
    QGraphicsOpacityEffect* opacityEffect;
    QPropertyAnimation* fadeAnimation;
};

#endif // LANGUAGESELECTIONPAGE_H 