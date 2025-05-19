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
    , currentLanguage("en")
{
    QSettings settings;
    const QString savedLanguage = settings.value("language", "en").toString();
    if (savedLanguage != "en") {
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
    if (translator->parent()) {
        qApp->removeTranslator(translator);
    }
    const QString qmFile = QString(":/translations/fitflexpro_%1.qm").arg(language);
    if (translator->load(qmFile)) {
        qDebug() << "Successfully loaded translation file:" << qmFile;
        qApp->installTranslator(translator);
        currentLanguage = language;
        QSettings settings;
        settings.setValue("language", language);
        emit languageChanged(language);
    } else {
        qDebug() << "Failed to load translation file:" << qmFile << ". Falling back to English.";
        if (language != "en" && translator->load(":/translations/fitflexpro_en.qm")) {
            qDebug() << "Successfully loaded English translation file";
            qApp->installTranslator(translator);
            currentLanguage = "en";
            QSettings settings;
            settings.setValue("language", "en");
            emit languageChanged("en");
        } else {
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