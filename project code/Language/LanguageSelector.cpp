#include "LanguageSelector.h"
#include "LanguageManager.h"
#include <QIcon>
#include <QDebug>
#include <QApplication>

LanguageSelector::LanguageSelector(QWidget* parent)
    : QComboBox(parent)
    , isDarkTheme(false)
{
    setupUI();
    
    // Connect to language manager
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LanguageSelector::onLanguageChanged);
    
    connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
            this, [this](const QString& language) {
                const int index = findData(language);
                if (index != -1 && index != currentIndex()) {
                    blockSignals(true);
                    setCurrentIndex(index);
                    blockSignals(false);
                }
            });
            
    // Set current language from settings
    const QString currentLang = LanguageManager::getInstance().getCurrentLanguage();
    const int index = findData(currentLang);
    if (index != -1) {
        setCurrentIndex(index);
    } else {
        // Default to English if no valid language is set
        const int englishIndex = findData("en");
        if (englishIndex != -1) {
            setCurrentIndex(englishIndex);
            LanguageManager::getInstance().setLanguage("en");
        }
    }
}

void LanguageSelector::setupUI()
{
    setFixedHeight(40);
    setCursor(Qt::PointingHandCursor);
    
    // Clear any existing items
    clear();
    
    // Add language options with flags (English first as default)
    addItem(QIcon(":/Images/en.png"), "English", "en");
    addItem(QIcon(":/Images/fr.png"), "Français", "fr");
    addItem(QIcon(":/Images/de.png"), "Deutsch", "de");
    
    updateTheme(isDarkTheme);
}

void LanguageSelector::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    setStyleSheet(QString(R"(
        QComboBox {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            border-radius: 8px;
            padding: 5px 10px;
            min-width: 120px;
            font-size: 14px;
        }
        QComboBox:hover {
            background-color: %4;
            border-color: %5;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox::down-arrow {
            image: url(:/Images/chevron-down.png);
            width: 12px;
            height: 12px;
        }
        QComboBox QAbstractItemView {
            background-color: %6;
            color: %7;
            border: 1px solid %8;
            border-radius: 8px;
            selection-background-color: %9;
            selection-color: %10;
        }
    )")
    .arg(isDark ? "#1F2937" : "#E1E9F5")  // Background
    .arg(isDark ? "#F9FAFB" : "#111827")  // Text color
    .arg(isDark ? "#374151" : "#E5E7EB")  // Border
    .arg(isDark ? "#374151" : "#F3F4F6")  // Hover background
    .arg(isDark ? "#4B5563" : "#D1D5DB")  // Hover border
    .arg(isDark ? "#1F2937" : "#E1E9F5")  // Dropdown background
    .arg(isDark ? "#F9FAFB" : "#111827")  // Dropdown text
    .arg(isDark ? "#374151" : "#E5E7EB")  // Dropdown border
    .arg(isDark ? "#4B5563" : "#EEF2FF")  // Selection background
    .arg(isDark ? "#FFFFFF" : "#111827")   // Selection text
    );
}

void LanguageSelector::onLanguageChanged(int index)
{
    if (index >= 0) {
        const QString languageCode = itemData(index).toString();
        qDebug() << "Language selected:" << languageCode;
        
        // Block signals to prevent feedback loop
        blockSignals(true);
        
        // Set the language
        LanguageManager::getInstance().setLanguage(languageCode);
        
        // Update combobox index if needed
        const int currentIndex = findData(languageCode);
        if (currentIndex != -1 && currentIndex != index) {
            setCurrentIndex(currentIndex);
        }
        
        // Unblock signals
        blockSignals(false);
        
        // Force immediate UI update
        QEvent event(QEvent::LanguageChange);
        QApplication::sendEvent(window(), &event);
        
        // Update parent widget if it exists
        if (parentWidget()) {
            QApplication::sendEvent(parentWidget(), &event);
        }
    }
} 