#ifndef MAINPAGESTYLE_H
#define MAINPAGESTYLE_H
#include <QString>

inline const QString stackedWidgetStyle = "QStackedWidget { background: transparent; }";

inline const QString titleLabelStyle = QString("QLabel { font-size: 20px; font-weight: 600; color: %1; }");

inline const QString smallButtonStyle = QString(
            "QPushButton {"
            "   background: transparent;"
            "   border: none;"
            "   color: #64748b;"
            "   font-size: 12px;"
            "   font-weight: 500;"
            "   padding: 6px 12px;"
            "   border-radius: 6px;"
            "   min-width: 80px;"
            "}"
            "QPushButton:hover {"
            "   background: rgba(139, 92, 246, 0.1);"
            "   color: #8B5CF6;"
            "}"
            "QPushButton:checked {"
            "   background: #8B5CF6;"
            "   color: white;"
            "}"
        );
inline const QString buttonStyle = QString(
            "QPushButton {"
            "   background: transparent;"
            "   border: none;"
            "   color: #64748b;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   padding: 8px 16px;"
            "   border-radius: 8px;"
            "   min-width: 100px;"
            "}"
            "QPushButton:hover {"
            "   background: rgba(139, 92, 246, 0.1);"
            "   color: #8B5CF6;"
            "}"
            "QPushButton:checked {"
            "   background: #8B5CF6;"
            "   color: white;"
            "}"
        );
// .args() is applied for dark theme in staffhomepage.cpp
inline const QString navBarStyler = QString(
        "QWidget#navBar {"
        "   background: %1;"
        "   backdrop-filter: blur(%2);"
        "   border-bottom: 1px solid rgba(243, 244, 246, 0.1);"
        "   box-shadow: 0 1px 2px rgba(0, 0, 0, 0.1);"
        "}"
    );


#endif //MAINPAGESTYLE_H
