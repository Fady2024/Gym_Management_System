#include "UIUtils.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

namespace UIUtils {
    QColor lerpColor(const QColor& color1, const QColor& color2, qreal t)
    {
        return QColor(
            color1.red() + t * (color2.red() - color1.red()),
            color1.green() + t * (color2.green() - color1.green()),
            color1.blue() + t * (color2.blue() - color1.blue()),
            color1.alpha() + t * (color2.alpha() - color1.alpha())
        );
    }

    QString getInputStyle(bool isDark)
    {
        if (isDark) {
            return QString(
                "QLineEdit {"
                "    background-color: #1a1b26;"
                "    border: 2px solid #2e2f3d;"
                "    border-radius: 12px;"
                "    padding-left: 44px;"
                "    padding-right: 44px;"
                "    padding-top: 0px;"
                "    padding-bottom: 0px;"
                "    margin: 0px;"
                "    color: #e2e8f0;"
                "    font-size: 14px;"
                "    min-width: 200px;"
                "    height: 45px;"
                "    line-height: 45px;"
                "    text-align: left;"
                "    vertical-align: middle;"
                "}"
                "QLineEdit:focus {"
                "    border: 2px solid #8B5CF6;"
                "    background-color: #1a1b26;"
                "}"
                "QLineEdit:hover {"
                "    border: 2px solid #7c3aed;"
                "    background-color: #1a1b26;"
                "}"
                "QLineEdit::placeholder {"
                "    color: #64748b;"
                "    vertical-align: middle;"
                "}"
            );
        } else {
            return QString(
                "QLineEdit {"
                "    background-color: #ffffff;"
                "    border: 2px solid #e2e8f0;"
                "    border-radius: 12px;"
                "    padding-left: 44px;"
                "    padding-right: 44px;"
                "    padding-top: 0px;"
                "    padding-bottom: 0px;"
                "    margin: 0px;"
                "    color: #1e293b;"
                "    font-size: 14px;"
                "    min-width: 200px;"
                "    height: 45px;"
                "    line-height: 45px;"
                "    text-align: left;"
                "    vertical-align: middle;"
                "}"
                "QLineEdit:focus {"
                "    border: 2px solid #8B5CF6;"
                "    background-color: #ffffff;"
                "}"
                "QLineEdit:hover {"
                "    border: 2px solid #7c3aed;"
                "    background-color: #ffffff;"
                "}"
                "QLineEdit::placeholder {"
                "    color: #94a3b8;"
                "    vertical-align: middle;"
                "}"
            );
        }
    }

    QString getButtonStyle(bool isDark)
    {
        return QString(
            "QPushButton {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #8B5CF6, stop:0.5 #7C3AED, stop:1 #6D28D9);"
            "   color: white;"
            "   border: none;"
            "   border-radius: 12px;"
            "   padding: 14px 28px;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   letter-spacing: 0.3px;"
            "   box-shadow: 0 4px 6px rgba(139, 92, 246, 0.2);"
            "   transition: all 0.3s ease;"
            "}"
            "QPushButton:hover {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #7C3AED, stop:0.5 #6D28D9, stop:1 #5B21B6);"
            "   box-shadow: 0 6px 10px rgba(139, 92, 246, 0.3);"
            "   transform: translateY(-1px);"
            "}"
            "QPushButton:pressed {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #6D28D9, stop:0.5 #5B21B6, stop:1 #4C1D95);"
            "   box-shadow: 0 2px 4px rgba(139, 92, 246, 0.2);"
            "   transform: translateY(1px);"
            "}"
            "QPushButton:disabled {"
            "   background: " + QString(isDark ? "#374151" : "#E5E7EB") + ";"
            "   color: " + QString(isDark ? "#6B7280" : "#9CA3AF") + ";"
            "   box-shadow: none;"
            "}"
        );
    }

    QString getCheckboxStyle(bool isDark)
    {
        return QString(
            "QCheckBox {"
            "   color: %1;"
            "   font-size: 14px;"
            "   spacing: 8px;"
            "}"
            "QCheckBox::indicator {"
            "   width: 22px;"
            "   height: 22px;"
            "   border: 2px solid %2;"
            "   border-radius: 6px;"
            "}"
            "QCheckBox::indicator:checked {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED);"
            "   border: 2px solid #8B5CF6;"
            "   image: url(:/Images/check.png);"
            "   image-position: center;"
            "}"
            "QCheckBox::indicator:hover {"
            "   border: 2px solid #8B5CF6;"
            "   background-color: rgba(139, 92, 246, 0.1);"
            "}"
            "QCheckBox::indicator:pressed {"
            "   background-color: rgba(139, 92, 246, 0.2);"
            "}"
        ).arg(
            isDark ? "#e2e8f0" : "#1e293b",
            isDark ? "#475569" : "#cbd5e1"
        );
    }


    QString getTabStyle(bool isDark, bool isActive)
    {
        return QString(
            "QPushButton {"
            "   background: %1;"
            "   color: %2;"
            "   border: none;"
            "   border-radius: 10px;"
            "   padding: 12px 24px;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   letter-spacing: 0.2px;"
            "   transition: all 0.2s ease;"
            "   box-shadow: %3;"
            "}"
            "QPushButton:hover {"
            "   background: %4;"
            "   box-shadow: %5;"
            "}"
        ).arg(
            isActive ? 
                "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED)" : 
                (isDark ? "#1F2937" : "#FFFFFF"),
            isActive ? "#FFFFFF" : (isDark ? "#F9FAFB" : "#111827"),
            isActive ? 
                "0 4px 6px rgba(139, 92, 246, 0.2)" : 
                (isDark ? "0 2px 4px rgba(0, 0, 0, 0.1)" : "0 2px 4px rgba(0, 0, 0, 0.05)"),
            isActive ? 
                "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7C3AED, stop:1 #6D28D9)" : 
                (isDark ? "#374151" : "#F3F4F6"),
            isActive ? 
                "0 6px 8px rgba(139, 92, 246, 0.25)" : 
                (isDark ? "0 4px 6px rgba(0, 0, 0, 0.15)" : "0 4px 6px rgba(0, 0, 0, 0.1)")
        );
    }

    QString getAuthContainerStyle(bool isDark) {
        return QString(
            "QWidget#authContainer {"
            "   background: %1;"
            "   border: 1px solid %2;"
            "   border-radius: 24px;"
            "   color: %3;"
            "   box-shadow: 0 10px 25px rgba(0, 0, 0, %4);"
            "}"
        ).arg(
            isDark ? "rgba(30, 41, 59, 0.95)" : "rgba(255, 255, 255, 0.95)",
            isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)",
            isDark ? "#e2e8f0" : "#1e293b",
            isDark ? "0.3" : "0.1"
        );
    }

    QString getWelcomeLabelStyle(bool isDark) {
        return QString(
            "QLabel {"
            "   font-size: 28px;"
            "   font-weight: 700;"
            "   color: %1;"
            "   letter-spacing: -0.5px;"
            "}"
        ).arg(isDark ? "#e2e8f0" : "#1e293b");
    }

    QString getSubtitleLabelStyle() {
        return "QLabel { color: #64748b; font-size: 15px; letter-spacing: 0.2px; }";
    }

    QString getProfileUploadStyle(bool isDark) {
        return QString(
            "QPushButton {"
            "   background-color: %1;"
            "   border: 2px dashed %2;"
            "   border-radius: 70px;"
            "   padding: 28px;"
            "   transition: all 0.3s ease;"
            "   box-shadow: 0 4px 6px rgba(0, 0, 0, 0.05);"
            "}"
            "QPushButton:hover {"
            "   background-color: %3;"
            "   border: 2px dashed %4;"
            "   box-shadow: 0 6px 8px rgba(0, 0, 0, 0.1);"
            "   transform: translateY(-2px);"
            "}"
            "QPushButton:pressed {"
            "   background-color: %5;"
            "   box-shadow: 0 2px 4px rgba(0, 0, 0, 0.05);"
            "   transform: translateY(1px);"
            "}"
        ).arg(
            isDark ? "#1e293b" : "#f1f5f9",
            isDark ? "#475569" : "#cbd5e1",
            isDark ? "#334155" : "#e2e8f0",
            isDark ? "#64748b" : "#94a3b8",
            isDark ? "#1e293b" : "#f1f5f9"
        );
    }

    QString getProfileUploadLabelStyle(bool isDark) {
        return QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: 15px;"
            "   font-weight: 500;"
            "   letter-spacing: 0.3px;"
            "   margin-top: 4px;"
            "}"
        ).arg(isDark ? "#64748b" : "#94a3b8");
    }

    QPixmap getIcon(const QString& name, int size) {
        const QString resourcePath = QString(":/Images/%1").arg(name.startsWith("Images/") ? name.mid(7) : name);
        QPixmap icon(resourcePath);
        
        if (icon.isNull()) {
            const QString directPath = name.startsWith("Images/") ? name : QString("Images/%1").arg(name);
            icon.load(directPath);
            
            if (icon.isNull()) {
                qDebug() << "Failed to load icon:" << name << "from both" << resourcePath << "and" << directPath;
                return QPixmap();
            }
        }
        
        return icon.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap getIconWithColor(const QString& name, const QColor& color, int size) {
        const QString resourcePath = QString(":/Images/%1").arg(name.startsWith("Images/") ? name.mid(7) : name);
        QPixmap icon(resourcePath);
        
        if (icon.isNull()) {
            const QString directPath = name.startsWith("Images/") ? name : QString("Images/%1").arg(name);
            icon.load(directPath);
            
            if (icon.isNull()) {
                qDebug() << "Failed to load colored icon:" << name << "from both" << resourcePath << "and" << directPath;
                return QPixmap();
            }
        }

        QPixmap result = icon.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPainter painter(&result);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(result.rect(), color);
        return result;
    }

    QString getMessageWidgetStyle(bool isDark, bool isError) {
        return QString(
            "QWidget#messageWidget {"
            "   background: %1;"
            "   border: 1px solid %2;"
            "   border-radius: 12px;"
            "   min-height: 48px;"
            "   box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);"
            "   padding: 8px 16px;"
            "}"
            "QLabel#messageText {"
            "   color: %3;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   line-height: 1.6;"
            "   margin: 0;"
            "   padding: 4px 0;"
            "   min-height: 24px;"
            "   text-align: center;"
            "   qproperty-alignment: AlignCenter;"
            "   qproperty-wordWrap: true;"
            "}"
        ).arg(
            isError ? 
                (isDark ? "rgba(239, 68, 68, 0.18)" : "rgba(239, 68, 68, 0.18)") :
                (isDark ? "rgba(34, 197, 94, 0.18)" : "rgba(34, 197, 94, 0.18)"),
            isError ?
                (isDark ? "rgba(239, 68, 68, 0.3)" : "rgba(239, 68, 68, 0.3)") :
                (isDark ? "rgba(34, 197, 94, 0.3)" : "rgba(34, 197, 94, 0.3)"),
            isError ?
                (isDark ? "#ef4444" : "#dc2626") :
                (isDark ? "#22c55e" : "#16a34a")
        );
    }

    QString getMessageIconStyle(bool isDark) {
        return QString(
            "QLabel {"
            "   background: transparent;"
            "   padding: 0;"
            "   margin: 0;"
            "   min-width: 24px;"
            "   max-width: 24px;"
            "   min-height: 24px;"
            "   max-height: 24px;"
            "   qproperty-alignment: AlignCenter;"
            "}"
        );
    }

} 