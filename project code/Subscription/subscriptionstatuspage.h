#ifndef SUBSCRIPTIONSTATUSPAGE_H
#define SUBSCRIPTIONSTATUSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QResizeEvent>

#include "../UI/UIUtils.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/userdatamanager.h"
#include "../Model/subscription.h"

// Forward declaration
class NewUserSubscriptionView;

class SubscriptionStatusPage : public QWidget
{
    Q_OBJECT

public:
    explicit SubscriptionStatusPage(QWidget *parent = nullptr, MemberDataManager* memberManager = nullptr, UserDataManager* userManager = nullptr);
    ~SubscriptionStatusPage();
    void setMemberManager(MemberDataManager* manager);
    void setUserManager(UserDataManager* manager);
    void setCurrentMemberId(int id);
    void updateTheme(bool isDark);
    void updateLayout();
    void retranslateUI();
    void loadMemberData();

signals:
    void subscribeRequested();
    void renewRequested(int planId, bool isVip);
    void changePlanRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override {
        if (event->type() == QEvent::LanguageChange) {
            retranslateUI();
        }
        QWidget::changeEvent(event);
    }

private:
    void setupUI();
    void createStatusCard();
    void createActionButtons();
    void updateStatusCard();
    void updateMemberInfo();
    void updateStatusColors();
    void updateRemainingDays();
    void setupCardStyles();
    void setupReturningMemberPromoView();
    void setupRegularView();
    QString getProgressBarStyle(int progressPercentage) const;

    // Helper methods
    QString getSubscriptionTypeName(SubscriptionType type);
    double getSubscriptionPrice(SubscriptionType type, bool isVip);

    QVBoxLayout* mainLayout;
    QStackedWidget* mainStack;
    QWidget* contentContainer;
    QLabel* titleLabel;
    QLabel* greetingLabel;
    QLabel* memberInfoLabel;
    QWidget* statusCard;
    QLabel* statusIconLabel;
    QLabel* statusTextLabel;
    QLabel* planTypeLabel;
    QLabel* datesLabel;
    QLabel* priceLabel;
    QProgressBar* daysProgressBar;
    QLabel* remainingDaysLabel;
    QWidget* actionsContainer;
    QPushButton* subscribeButton;
    QPushButton* renewButton;
    QPushButton* changePlanButton;
    QLabel* historyLabel;
    
    MemberDataManager* memberManager;
    UserDataManager* userManager;
    int currentMemberId;
    bool isDarkTheme;
    
    // Cached subscription data
    bool hasActiveSubscription;
    bool isExpiringSoon;
    int daysRemaining;
    QString memberName;
    QDate startDate;
    QDate endDate;
    SubscriptionType currentPlan;
    bool isVipMember;

    // New user subscription view
    NewUserSubscriptionView* newUserView;
};

// New class for new user subscription view
class NewUserSubscriptionView : public QWidget
{
    Q_OBJECT

public:
    explicit NewUserSubscriptionView(QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void updateLayout();
    void retranslateUI();

protected:
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QWidget::event(e);
    }

    void changeEvent(QEvent* event) override {
        if (event->type() == QEvent::LanguageChange) {
            retranslateUI();
        }
        QWidget::changeEvent(event);
    }

signals:
    void subscribeRequested();

private:
    void setupUI();
    void setupCardStyles();

    QVBoxLayout* mainLayout;
    QWidget* contentContainer;
    QWidget* statusCard;
    QLabel* emojiLabel;
    QLabel* headingLabel;
    QLabel* subtitleLabel;
    QFrame* divider;
    QWidget* benefitsContainer;
    QWidget* pricingContainer;
    QPushButton* subscribeButton;
    QLabel* noteLabel;
    
    bool isDarkTheme;
};

#endif // SUBSCRIPTIONSTATUSPAGE_H 