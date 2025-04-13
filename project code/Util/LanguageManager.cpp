#include "LanguageManager.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>

LanguageManager* LanguageManager::instance = nullptr;

LanguageManager& LanguageManager::getInstance()
{
    if (!instance) {
        instance = new LanguageManager();
    }
    return *instance;
}

LanguageManager::LanguageManager()
    : translator(new QTranslator)
    , currentLanguage("en")  // Initialize with English
{
    // Load saved language preference
    QSettings settings;
    const QString savedLanguage = settings.value("language", "en").toString();
    if (savedLanguage != "en") {  // Only load translator if not English
        setLanguage(savedLanguage);
    }
}

LanguageManager::~LanguageManager()
{
    delete translator;
}

void LanguageManager::setLanguage(const QString& language)
{
    if (currentLanguage == language)
        return;

    // Always remove current translator first
    if (translator->parent()) {
        qApp->removeTranslator(translator);
    }

    // Load translation file for the selected language
    const QString qmFile = QString(":/translations/fitflexpro_%1.qm").arg(language);
    if (translator->load(qmFile)) {
        qDebug() << "Successfully loaded translation file:" << qmFile;
        qApp->installTranslator(translator);
        currentLanguage = language;

        // Save language preference
        QSettings settings;
        settings.setValue("language", language);

        // Emit signal for UI update
        emit languageChanged(language);
    } else {
        qDebug() << "Failed to load translation file:" << qmFile << ". Falling back to English.";
        // Try to load English translation file
        if (language != "en" && translator->load(":/translations/fitflexpro_en.qm")) {
            qDebug() << "Successfully loaded English translation file";
            qApp->installTranslator(translator);
            currentLanguage = "en";
            QSettings settings;
            settings.setValue("language", "en");
            emit languageChanged("en");
        } else {
            // If English translation fails, remove translator completely
            if (translator->parent()) {
                qApp->removeTranslator(translator);
            }
            currentLanguage = "en";
            QSettings settings;
            settings.setValue("language", "en");
            emit languageChanged("en");
        }
    }
} 