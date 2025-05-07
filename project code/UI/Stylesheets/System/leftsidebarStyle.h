#ifndef LEFTSIDEBARSTYLE_H
#define LEFTSIDEBARSTYLE_H
#include <QString>

inline const QString leftsidebarStyle = R"(
        QWidget#leftSidebar {
            background: %1;
            border-right: 1px solid %2;
        }
    )";

#endif //LEFTSIDEBARSTYLE_H
