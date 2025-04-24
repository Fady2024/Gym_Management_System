#ifndef SUBSCRIPTIONPAGE_H
#define SUBSCRIPTIONPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QSizePolicy>
#include <QScrollArea>
#include <QCheckBox>
#include <QEvent>
#include <QResizeEvent>
#include <QMessageBox>

#include "../UI/UIUtils.h"
#include "../DataManager/memberdatamanager.h"
#include "../Model/subscription.h"

class SubscriptionPage : public QWidget
{
    Q_OBJECT

public:
    explicit SubscriptionPage(QWidget *parent = nullptr, MemberDataManager* memberManager = nullptr);
    void updateTheme(bool isDark);
    void updateLayout();
    void retranslateUI();
    void updatePlanCardTexts();
    void setCurrentMemberId(int memberId);

signals:
    void paymentRequested(int planId, bool isVip);
    void subscriptionCompleted();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override {
        if (event->type() == QEvent::LanguageChange) {
            retranslateUI();
        }
        QWidget::changeEvent(event);
    }

private slots:
    void onVipToggleChanged(int cardId, bool checked);
    void onPlanSelected(int id);
    void onSelectButtonClicked(int id);
    void showVipConfirmationDialog(int planId);
    void handlePaymentCompleted(int planId, bool isVip);

private:
    void setupUI();
    void createPlanCard(const QString& title, const QString& price, const QString& duration,
                       const QStringList& features, int id, bool isPopular = false);
    void updateCardStyles();
    void animateCardSelection(QWidget* card, bool selected);
    void updateVIPState(int cardId, bool isVip);
    void updateCardStyle(QWidget* card, int cardId, bool isVip, bool isSelected);
    void updateCardLayout(QWidget* card, int cardWidth);
    void createSubscription(int planId, bool isVip);

    QVBoxLayout* mainLayout{};
    QWidget* contentContainer{};
    QLabel* titleLabel{};
    QWidget* vipToggleContainer{};
    QButtonGroup* planButtonGroup{};
    QHBoxLayout* plansLayout{};
    QMap<int, QWidget*> planCards;
    QMap<int, QCheckBox*> vipToggles;
    QMap<int, QLabel*> vipEncouragements;
    bool isDarkTheme;
    bool isVipEnabled{};
    int selectedPlanId;
    int currentVipCardId;
    bool isUpdatingStyles;
    bool isLayoutUpdatePending;
    QMessageBox* vipConfirmationDialog{};
    MemberDataManager* memberManager{};
    int currentMemberId{};
};

#endif // SUBSCRIPTIONPAGE_H