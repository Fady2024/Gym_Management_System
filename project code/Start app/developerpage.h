#ifndef DEVELOPERPAGE_H
#define DEVELOPERPAGE_H

#include <qevent.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>
#include <QParallelAnimationGroup>

class DeveloperPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeveloperPage(QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void updateLayout();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void scrollLeft();
    void scrollRight();
    void autoScroll();
    void stopAutoScroll();
    void resumeAutoScroll();

private:
    void setupUI();
    void createTeamMemberCard(const QString& name, const QString& role, const QString& imagePath = ":/Images/person.png", const QString& githubUsername = "");
    void updateCardStyles();
    bool handleTouchEvent(QTouchEvent* event);
    void updateCardPositions();
    void applyCardEffects(QWidget* card, bool isCenter = false);
    void centerCardsInView();
    void SideCardTextAlignment(QWidget* card);

    QHBoxLayout* mainLayout;
    QWidget* cardsContainer;
    QHBoxLayout* cardsLayout;
    QPushButton* leftButton;
    QPushButton* rightButton;
    QTimer* autoScrollTimer;
    bool isDarkTheme;
    int currentIndex;
    int cardWidth;
    int cardHeight;
    int cardSpacing;
    QList<QWidget*> teamCards;
    bool isFirstShow;
    QPoint mousePressPos;
    bool isDragging = false;
};

#endif // DEVELOPERPAGE_H

