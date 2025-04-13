#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QSettings>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& getInstance() {
        static ThemeManager instance;
        return instance;
    }

    bool isDarkTheme() const { return m_isDarkTheme; }
    void setDarkTheme(bool isDark);
    void initializeTheme();

signals:
    void themeChanged(bool isDark);

private:
    ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void applyThemeToApplication(bool isDark);

    bool m_isDarkTheme;
    QSettings m_settings;
};

#endif // THEMEMANAGER_H 