#ifndef ADDMEMBERPAGE_H
#define ADDMEMBERPAGE_H

#include <QMainWindow>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include "../Theme/ThemeManager.h"
#include "../Language/LanguageManager.h"
#include "UIUtils.h"
#include "UserDataManager.h"
#include "memberdatamanager.h"
#include "TopPanel.h"

class AddMemberPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddMemberPage(UserDataManager* userDataManager,MemberDataManager* memberDataManger, QWidget* parent = nullptr);
    ~AddMemberPage() = default;

signals:
    void memberAdded(const QString& email);

private slots:
    void handleAddMember();
    void selectProfileImage();
    void retranslateUI();

private:
    void setupUI();
    void setupGlassEffect();
    void setupMessageWidget();
    void updateTheme(bool isDark);
    void updateAllTextColors();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void animateMessageWidget(bool isError);
    void clearFields();
    void updateLayout();

    bool isDarkTheme;
    UserDataManager* userDataManager;
    MemberDataManager* memberDataManger;
    QGraphicsOpacityEffect* opacityEffect;
    QLineEdit* nameInput;
    QLineEdit* emailInput;
    QLineEdit* passwordInput;
    QDateEdit* dateOfBirthInput;
    QPushButton* profileImageButton;
    QString selectedImagePath;
    QPixmap lastCircularPhoto;
    QPushButton* addMemberButton;
    QWidget* messageWidget;
    QLabel* messageText;
    QLabel* messageIcon;
    QTimer* messageTimer;
};

#endif // ADDMEMBERPAGE_H