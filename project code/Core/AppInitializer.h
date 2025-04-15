#ifndef APPINITIALIZER_H
#define APPINITIALIZER_H

#include <QtGlobal>
#include <QObject>
class AppInitializer : public QObject
{
    Q_OBJECT
    
public:
    static AppInitializer& getInstance() {
        static AppInitializer instance;
        return instance;
    }
    static bool initializeApplication();
    
    static void configurePerformance();

private:
    AppInitializer() : QObject(nullptr) {}
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // APPINITIALIZER_H 