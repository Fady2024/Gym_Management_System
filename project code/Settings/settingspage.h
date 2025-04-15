#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QDialog>
#include <QStackedWidget>
#include <QScrollArea>
#include <functional>
#include <QEvent>

#include "../DataManager/userdatamanager.h"
#include "developerpage.h"

// Forward declaration
class MainPage;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(UserDataManager* userDataManager, QWidget *parent = nullptr);
    void updateTheme(bool isDark);
    void updateLayout();
    void loadUserData(const QString& email = QString());
    void retranslateUI();

public slots:
    void onUserDataLoaded(const QString& email);

signals:
    void logoutRequested();
    void accountDeleted();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QWidget::event(e);
    }

private slots:
    void changeProfilePicture();
    void saveChanges();
    void handleLogout();
    void handleDeleteAccount();
    void resetChanges();
    void handleSettingsTab();
    void handleDeveloperTab();
    void updateTabButtons();

private:
    void setupUI();
    void showConfirmationDialog(const QString& title, const QString& message, 
        const QString& confirmText, const QString& cancelText, std::function<void()> onConfirm);
    static void updateSuccessDialogTheme(QDialog* dialog, bool isDark);
    void resetToDefaultProfileImage();
    void showMessageDialog(const QString& message, bool isError = false);

    UserDataManager* userDataManager;
    QHBoxLayout* mainLayout;
    QVBoxLayout* leftLayout;
    QStackedWidget* contentStack;
    QWidget* settingsContent;
    DeveloperPage* developerPage;
    QPushButton* settingsTabButton;
    QPushButton* developerTabButton;
    QPushButton* profileImageButton;
    QLineEdit* nameEdit;
    QLineEdit* emailEdit;
    QPushButton* saveButton;
    QPushButton* resetButton;
    QPushButton* changePictureButton;
    QPushButton* logoutButton;
    QPushButton* deleteAccountButton;
    QString currentPhotoPath;
    bool isDarkTheme;  // Track current theme state
};

#endif // SETTINGSPAGE_H 