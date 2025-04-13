#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QVBoxLayout>
#include <QTimer>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include "../Util/ThemeManager.h"

class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget* parent = nullptr);
    void startAnimation();

signals:
    void animationFinished();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;

private:
    void setupUI();
    void setupAnimations();
    void updateTheme(bool isDark);
    void retranslateUI();
    void createLoadingAnimation();

    QWidget* container;
    QLabel* logoLabel;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QWidget* loadingDots;
    QList<QLabel*> dotLabels;

    QGraphicsOpacityEffect* opacityEffect;
    QPropertyAnimation* fadeAnimation;
    QPropertyAnimation* scaleAnimation;
    QPropertyAnimation* slideAnimation;
    QSequentialAnimationGroup* mainAnimation;
    QParallelAnimationGroup* dotsAnimation;
};

#endif // SPLASHSCREEN_H 