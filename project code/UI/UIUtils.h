#ifndef UIUTILS_H
#define UIUTILS_H

#include <QIcon>

namespace UIUtils {
    // Core color and image manipulation functions
    QColor lerpColor(const QColor& color1, const QColor& color2, qreal t);

    // Theme-aware style functions
    QString getInputStyle(bool isDark);
    QString getButtonStyle(bool isDark);
    QString getCheckboxStyle(bool isDark);
    QString getTabStyle(bool isDark, bool isActive);

    // Authentication UI styles
    QString getAuthContainerStyle(bool isDark);
    QString getWelcomeLabelStyle(bool isDark);
    QString getSubtitleLabelStyle();
    QString getProfileUploadStyle(bool isDark);
    QString getProfileUploadLabelStyle(bool isDark);

    // Icon management
    QPixmap getIcon(const QString& name, int size = 20);
    QPixmap getIconWithColor(const QString& name, const QColor& color, int size = 20);

    // UI component styles
    QString getMessageWidgetStyle(bool isDark, bool isError);
    QString getMessageIconStyle(bool isDark);
}

#endif // UIUTILS_H 