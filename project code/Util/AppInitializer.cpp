#include "AppInitializer.h"
#include <QDebug>
#include <QApplication>
#include <QDir>

bool AppInitializer::initializeApplication() {
    qputenv("QT_STYLE_OVERRIDE", "");
    qputenv("QT_LOGGING_RULES", "*=false");
    qputenv("QT_MESSAGE_PATTERN", "");
    
    qInstallMessageHandler(messageHandler);
    
    configurePerformance();
    
    return true;
}

void AppInitializer::configurePerformance() {
    QApplication::setEffectEnabled(Qt::UI_AnimateCombo, true);
    QApplication::setEffectEnabled(Qt::UI_FadeMenu, true);
    QApplication::setEffectEnabled(Qt::UI_FadeTooltip, true);

}

void AppInitializer::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
    
}