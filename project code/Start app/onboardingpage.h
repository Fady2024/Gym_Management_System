#ifndef ONBOARDINGPAGE_H
#define ONBOARDINGPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEvent>

class OnboardingPage : public QWidget
{
    Q_OBJECT

public:
    explicit OnboardingPage(QWidget* parent = nullptr);
    void startAnimation();
    void updateTheme(bool isDark);

signals:
    void onboardingCompleted();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override;

private slots:
    void onNextClicked();
    void onSkipClicked();
    void onBackClicked();

private:
    void setupUI();
    void setupAnimations();
    void updatePage(int pageIndex);
    void loadAndScaleImage(const QString& imagePath);
    void retranslateUI();

    QWidget* container;
    QWidget* topBar;
    QLabel* imageLabel;
    QLabel* titleLabel;
    QLabel* descriptionLabel;
    QPushButton* skipButton;
    QPushButton* nextButton;
    QPushButton* backButton;
    QWidget* progressDotsContainer;
    QList<QLabel*> progressDots;

    QGraphicsOpacityEffect* opacityEffect;
    QPropertyAnimation* fadeAnimation;
    QPropertyAnimation* slideAnimation;
    int currentPage;

    QStringList imagePaths = {
        ":/Images/onboarding1.png",
        ":/Images/onboarding2.png",
        ":/Images/onboarding3.png"
    };
};

#endif // ONBOARDINGPAGE_H 