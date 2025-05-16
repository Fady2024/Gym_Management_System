#include "ThemeManager.h"
#include <QDebug>
#include <QApplication>
#include <QPalette>

ThemeManager::ThemeManager(QObject* parent) 
    : QObject(parent)
    , m_isDarkTheme(true)
{
    m_isDarkTheme = true;
    m_settings.setValue("darkTheme", true);
    m_settings.sync();
    
    // Apply the dark theme
    applyThemeToApplication(true);
    emit themeChanged(true);
    
    qDebug() << "ThemeManager initialized with dark theme";
}

void ThemeManager::initializeTheme() 
{
    // Always set to dark theme when initializing
    if (!m_isDarkTheme) {
        m_isDarkTheme = true;
        m_settings.setValue("darkTheme", true);
        m_settings.sync();
        applyThemeToApplication(true);
        emit themeChanged(true);
        qDebug() << "Theme initialized to dark mode";
    }
}

void ThemeManager::setDarkTheme(bool isDark) 
{
    if (m_isDarkTheme != isDark) {
        m_isDarkTheme = isDark;
        m_settings.setValue("darkTheme", isDark);
        m_settings.sync();
        applyThemeToApplication(isDark);
        emit themeChanged(isDark);
        qDebug() << "Theme changed to:" << (isDark ? "Dark" : "Light");
    }
}

void ThemeManager::applyThemeToApplication(bool isDark)
{
    QApplication* app = qApp;
    if (!app) return;

    QPalette palette;
    if (isDark) {
        palette.setColor(QPalette::Window, QColor("#1A1F2C"));
        palette.setColor(QPalette::WindowText, QColor("#F9FAFB"));
        palette.setColor(QPalette::Base, QColor("#1F2937"));
        palette.setColor(QPalette::AlternateBase, QColor("#374151"));
        palette.setColor(QPalette::Text, QColor("#F9FAFB"));
        palette.setColor(QPalette::Button, QColor("#1F2937"));
        palette.setColor(QPalette::ButtonText, QColor("#F9FAFB"));
        palette.setColor(QPalette::Highlight, QColor("#7E69AB"));
        palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
        palette.setColor(QPalette::Link, QColor("#7E69AB"));
        palette.setColor(QPalette::LinkVisited, QColor("#7E69AB"));
        palette.setColor(QPalette::ToolTipBase, QColor("#1F2937"));
        palette.setColor(QPalette::ToolTipText, QColor("#F9FAFB"));
        palette.setColor(QPalette::PlaceholderText, QColor("#6B7280"));
    } else {
        palette.setColor(QPalette::Window, QColor("#F0F4FA"));
        palette.setColor(QPalette::WindowText, QColor("#111827"));
        palette.setColor(QPalette::Base, QColor("#E1E9F5"));
        palette.setColor(QPalette::AlternateBase, QColor("#F3F4F6"));
        palette.setColor(QPalette::Text, QColor("#111827"));
        palette.setColor(QPalette::Button, QColor("#E1E9F5"));
        palette.setColor(QPalette::ButtonText, QColor("#111827"));
        palette.setColor(QPalette::Highlight, QColor("#7E69AB"));
        palette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
        palette.setColor(QPalette::Link, QColor("#7E69AB"));
        palette.setColor(QPalette::LinkVisited, QColor("#7E69AB"));
        palette.setColor(QPalette::ToolTipBase, QColor("#E1E9F5"));
        palette.setColor(QPalette::ToolTipText, QColor("#111827"));
        palette.setColor(QPalette::PlaceholderText, QColor("#9CA3AF"));
    }

    app->setPalette(palette);
} 