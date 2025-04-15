#ifndef AUTHPAGE_H
#define AUTHPAGE_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QEvent>
#include <QComboBox>
#include <QDateEdit>
#include "../DataManager/userdatamanager.h"
#include "imageslider.h"
#include "../UI/UIUtils.h"
#include "../Language//LanguageManager.h"

class CustomLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {}

signals:
    void focusGained();

protected:
    void focusInEvent(QFocusEvent* event) override {
        QLineEdit::focusInEvent(event);
        emit focusGained();
    }
};

class AuthPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuthPage(UserDataManager* userDataManager, QWidget* parent = nullptr);
    void onThemeToggled(bool darkTheme);
    void retranslateUI();

signals:
    void loginSuccessful(const QString& email);

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QMainWindow::event(e);
    }

private slots:
    void toggleForm();
    void togglePasswordVisibility() const;
    void handleLogin();
    void handleSignup();
    void selectProfileImage();
    void toggleTheme();
    void showTermsOfService();

private:
    void setupUI();
    void setupLoginForm();
    void setupSignupForm();
    void setupImageSlider();
    void updateTheme(bool isDark);
    void showError(const QString& message) const;
    void showSuccess(const QString& message) const;
    void clearFields();
    void setupGlassEffect() const;
    void animateTabSwitch(int index, QPushButton* loginTab, QPushButton* signupTab);
    void updateAllTextColors() const;
    void updateLayout() const;
    void setupMessageWidget();
    void animateMessageWidget(bool isError) const;
    void showConfirmationDialog(const QString& title, const QString& message,
        const QString& confirmText, const QString& cancelText, std::function<void()> onConfirm);

    UserDataManager* userDataManager;
    QStackedWidget* stackedWidget{};
    QWidget* loginWidget{};
    QWidget* signupWidget{};

    CustomLineEdit* loginEmailInput{};
    CustomLineEdit* loginPasswordInput{};
    QPushButton* loginButton{};
    QCheckBox* rememberMeCheckbox{};
    QPushButton* forgotPasswordButton{};

    CustomLineEdit* signupNameInput{};
    CustomLineEdit* signupEmailInput{};
    CustomLineEdit* signupPasswordInput{};
    CustomLineEdit* signupAgeInput{};
    QPushButton* profileImageButton{};
    QCheckBox* termsCheckbox{};
    QPushButton* signupButton{};

    // Message widget
    QWidget* messageWidget{};
    QLabel* messageIcon{};
    QLabel* messageText{};
    QSequentialAnimationGroup* messageAnimation{};
    QPropertyAnimation* shakeAnimation{};
    QPropertyAnimation* slideAnimation{};
    QPropertyAnimation* fadeAnimation{};
    QTimer* messageTimer{};

    bool isDarkTheme;

    QString selectedImagePath;
    QPixmap lastCircularPhoto;

    QGraphicsOpacityEffect* opacityEffect;
    ImageSlider* imageSlider{};
};

#endif 