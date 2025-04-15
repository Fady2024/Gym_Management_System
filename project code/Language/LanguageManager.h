#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    static LanguageManager& getInstance();
    void setLanguage(const QString& language);
    QString getCurrentLanguage() const { return currentLanguage; }

signals:
    void languageChanged(const QString& language);

private:
    LanguageManager();
    ~LanguageManager() override;
    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;

    static LanguageManager* instance;
    QTranslator* translator;
    QString currentLanguage;
};

#endif // LANGUAGEMANAGER_H 