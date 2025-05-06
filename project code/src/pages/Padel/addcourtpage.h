#ifndef ADDCOURTPAGE_H
#define ADDCOURTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QPushButton>
#include <QTimeEdit>
#include <QListWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QMap>
#include <QSpinBox>

#include "../../../DataManager/padeldatamanager.h"
#include "../../../UI/UIUtils.h"

class FeatureCard : public QWidget {
    Q_OBJECT
public:
    FeatureCard(const QString& icon, const QString& text, QWidget* parent = nullptr);
    bool isSelected() const { return selected; }
    void setSelected(bool value);
    QString getText() const { return featureText; }
    void setIsDarkTheme(bool isDark);

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* iconLabel;
    QLabel* textLabel;
    bool selected;
    bool isDarkTheme;
    QString featureText;
};

class TimeSlotButton : public QPushButton {
    Q_OBJECT
public:
    TimeSlotButton(const QString& text, QWidget* parent = nullptr);
};

class AddCourtPage : public QWidget
{
    Q_OBJECT

public:
    explicit AddCourtPage(PadelDataManager* padelManager, QWidget* parent = nullptr);
    ~AddCourtPage();

    void updateTheme(bool isDark);
    void retranslateUI();
    void updateLayout();

protected:
    void resizeEvent(QResizeEvent* event) override {
        updateLayout();
        QWidget::resizeEvent(event);
    }

private slots:
    void onAddCourtClicked();
    void onCancelClicked();
    void onAddFeatureClicked();
    void onFeatureToggled(bool checked);
    void onTimeSlotToggled();
    void updateCharCount();
    void onCourtTypeToggled(bool checked);

private:
    void setupUI();
    void setupConnections();
    void setupFeatureCards();
    void setupTimeSlotButtons();
    void validateField(QLineEdit* field, bool valid, const QString& errorMsg = QString());
    void clearForm();
    void showSuccessMessage(const QString& message);
    void showErrorMessage(const QString& message);

    // UI Elements
    QVBoxLayout* mainLayout;
    QScrollArea* scrollArea;
    QWidget* contentWidget;
    
    // Basic Info Elements
    QLineEdit* nameEdit;
    QLabel* nameErrorLabel;
    QComboBox* locationComboBox;
    QLabel* locationErrorLabel;
    QRadioButton* indoorRadio;
    QRadioButton* outdoorRadio;
    QButtonGroup* courtTypeGroup;
    QDoubleSpinBox* priceSpinBox;
    QLabel* priceErrorLabel;
    QSpinBox* attendeesSpinBox;
    QLabel* attendeesLabel;
    
    // Features Elements
    QWidget* featuresContainer;
    QLineEdit* featureEdit;
    QPushButton* addFeatureButton;
    QMap<QString, FeatureCard*> featureCards;
    QStringList selectedFeatures;
    
    // Time Slots Elements
    QWidget* timeSlotContainer;
    QMap<QString, TimeSlotButton*> timeSlotButtons;
    QStringList selectedTimeSlots;
    QLabel* timeSlotErrorLabel;
    
    // Description Elements
    QTextEdit* descriptionEdit;
    QLabel* charCountLabel;
    
    // Action Buttons
    QPushButton* addCourtButton;
    QPushButton* cancelButton;
    
    // Data Management
    PadelDataManager* padelDataManager;
    bool isDarkTheme;
};

#endif // ADDCOURTPAGE_H 