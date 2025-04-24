#ifndef PAYMENTPAGE_H
#define PAYMENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QSizePolicy>
#include <QScrollArea>
#include <QEvent>
#include <QResizeEvent>
#include <QDate>
#include <QMessageBox>
#include <QRegularExpression>
#include <QCheckBox>

#include "../../UI/UIUtils.h"
#include "../../Theme/ThemeManager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/userdatamanager.h"
#include "../Model/subscription.h"

class PaymentPage : public QWidget
{
    Q_OBJECT

public:
    explicit PaymentPage(QWidget *parent = nullptr, MemberDataManager* memberManager = nullptr, UserDataManager* userDataManager = nullptr);
    void updateTheme(bool isDark);
    void updateLayout();
    void retranslateUI();
    void setPlanDetails(int planId, bool isVip);
    void setCurrentMemberId(int memberId);
    void setCurrentUserId(int userId) { currentUserId = userId; }
    void setUserDataManager(UserDataManager* manager) { userDataManager = manager; }
    void loadSavedCardData();

signals:
    void paymentCompleted(int planId, bool isVip);
    void backToSubscription();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onPayButtonClicked();
    void onBackButtonClicked();
    void showCVCInfo();
    void validateExpiryDate(const QString& text);
    void processPayment();
    void toggleSavedCardUsage(bool checked);

private:
    void setupUI();
    void createPaymentForm();
    void updateCardStyles();
    void updateInputStyles();
    void updateButtonStyles();
    void updateTitleStyle();
    void setupCVCDialog();
    bool validatePaymentDetails();
    bool createSubscription();
    void populateFieldsWithSavedCard();

    QVBoxLayout* mainLayout;
    QWidget* contentContainer;
    QLabel* titleLabel;
    QWidget* paymentCard;
    QLineEdit* cardNumberInput;
    QLineEdit* expiryDateInput;
    QLineEdit* cvvInput;
    QLineEdit* nameInput;
    QPushButton* payButton;
    QPushButton* backButton;
    QLabel* planPriceLabel;
    QLabel* vipPriceLabel;
    QLabel* totalPriceLabel;
    QComboBox* monthCombo;
    QComboBox* yearCombo;
    QMessageBox* cvcDialog;
    MemberDataManager* memberManager;
    UserDataManager* userDataManager;
    int currentMemberId;
    int selectedPlanId;
    bool isVipEnabled;
    bool isDarkTheme;
    bool isUpdatingStyles;
    bool isLayoutUpdatePending;
    int currentUserId;

    // Saved card data
    QCheckBox* useSavedCardCheckbox;
    QWidget* savedCardInfoContainer;
    QLabel* savedCardLabel;
    bool hasSavedCard;
    SavedCardData savedCardData;
};

#endif // PAYMENTPAGE_H 