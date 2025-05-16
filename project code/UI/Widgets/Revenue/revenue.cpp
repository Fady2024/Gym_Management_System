#include "revenue.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include "../DataManager/userdatamanager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/classdatamanager.h"
#include "../DataManager/padeldatamanager.h"

Revenue::Revenue(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
                ClassDataManager* classDataManager, QWidget* parent)
    : QWidget(parent)
    , isDarkTheme(false)
    , userManager(userDataManager)
    , memberManager(memberDataManager)
    , classManager(classDataManager)
{
    setupUI();

}

void Revenue::setupUI()
{
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Title
    QLabel* title = new QLabel("Revenue Analytics", this);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Create metrics sections
    auto createMetricSection = [](const QString& title) {
        QWidget* widget = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setSpacing(4);
        layout->setContentsMargins(8, 8, 8, 8);
        
        QLabel* label = new QLabel(title);
        QProgressBar* bar = new QProgressBar;
        bar->setFixedHeight(4);
        bar->setTextVisible(false);
        
        layout->addWidget(label);
        layout->addWidget(bar);
        return std::make_tuple(widget, label, bar);
    };

    // Create metrics
    auto [membershipWidget, mLabel, mBar] = createMetricSection("Membership");
    auto [classWidget, cLabel, cBar] = createMetricSection("Classes");
    auto [courtWidget, coLabel, coBar] = createMetricSection("Courts");
    auto [growthWidget, gLabel, gBar] = createMetricSection("Growth");

    membershipLabel = mLabel;
    classLabel = cLabel;
    courtLabel = coLabel;
    growthLabel = gLabel;
    membershipBar = mBar;
    classBar = cBar;
    courtBar = coBar;
    growthBar = gBar;

    // Add metrics in 2x2 grid using simple layouts
    QHBoxLayout* row1 = new QHBoxLayout;
    row1->addWidget(membershipWidget);
    row1->addWidget(classWidget);
    
    QHBoxLayout* row2 = new QHBoxLayout;
    row2->addWidget(courtWidget);
    row2->addWidget(growthWidget);

    mainLayout->addLayout(row1);
    mainLayout->addLayout(row2);
    mainLayout->addStretch();

    updateTheme(isDarkTheme);
}

void Revenue::updateMetrics(QLabel* label, QProgressBar* bar, const QString& text, int value)
{
    if (label && bar) {
        label->setText(text);
        bar->setValue(value);
    }
}

// void Revenue::updateData()
// {
//     if (!memberManager || !classManager || !userManager) return;
//
//     // Membership metrics
//     QVector<User> allUsers = userManager->getAllUsers();
//     int totalMembers = 0, vipMembers = 0;
//     for (const User& user : allUsers) {
//         if (memberManager->userIsMember(user.getId())) {
//             totalMembers++;
//             if (memberManager->isVIPMember(user.getId())) vipMembers++;
//         }
//     }
//     int vipPercentage = totalMembers > 0 ? (vipMembers * 100) / totalMembers : 0;
//     updateMetrics(membershipLabel, membershipBar,
//         QString("Members: %1\nVIP: %2%").arg(totalMembers).arg(vipPercentage),
//         vipPercentage);
//
//     // Class metrics
//     QVector<Class> allClasses = classManager->getAllClasses();
//     int totalAttendance = 0, totalCapacity = 0;
//     QDate currentDate = QDate::currentDate();
//     QDate monthStart = QDate(currentDate.year(), currentDate.month(), 1);
//
//     for (const Class& gymClass : allClasses) {
//         QVector<AttendanceRecord> records = classManager->getAttendanceRecords(
//             gymClass.getId(), monthStart, currentDate);
//         for (const AttendanceRecord& record : records) {
//             if (record.attended) totalAttendance++;
//             totalCapacity++;
//         }
//     }
//
//     int attendanceRate = totalCapacity > 0 ? (totalAttendance * 100) / totalCapacity : 0;
//     updateMetrics(classLabel, classBar,
//         QString("Classes: %1\nAttendance: %2%").arg(allClasses.size()).arg(attendanceRate),
//         attendanceRate);
//
//     // Court metrics
//     int totalBookings = 0, vipBookings = 0;
//     double revenue = 0;
//     QVector<Booking> bookings = padelManager->getBookingsByDate(QDate::currentDate());
//     for (const Booking& booking : bookings) {
//         if (!booking.isCancelled()) {
//             totalBookings++;
//             if (booking.isVip()) vipBookings++;
//             revenue += booking.getPrice();
//         }
//     }
//     int vipRate = totalBookings > 0 ? (vipBookings * 100) / totalBookings : 0;
//     updateMetrics(courtLabel, courtBar,
//         QString("Bookings: %1\nRevenue: $%2").arg(totalBookings).arg(revenue, 0, 'f', 2),
//         vipRate);
//
//     // Growth metrics
//     int previousMembers = 0;
//     QDate lastMonth = QDate::currentDate().addMonths(-1);
//     for (const User& user : allUsers) {
//         if (memberManager->userIsMember(user.getId())) {
//             const Member& member = memberManager->getMemberByUserId(user.getId());
//             if (member.getSubscription().getStartDate() < lastMonth) {
//                 previousMembers++;
//             }
//         }
//     }
//     int growthRate = previousMembers > 0 ? ((totalMembers - previousMembers) * 100) / previousMembers : 0;
//     updateMetrics(growthLabel, growthBar,
//         QString("Growth: %1%\nNew: %2").arg(growthRate).arg(totalMembers - previousMembers),
//         growthRate > 0 ? growthRate : 0);
// }

void Revenue::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    QString baseStyle = isDark ? 
        "background-color: #1F2937; color: #F9FAFB; border: 1px solid #374151;" :
        "background-color: #F3F4F6; color: #111827; border: 1px solid #E5E7EB;";
    
    QString barStyle = isDark ?
        "QProgressBar { background: #374151; border: none; } "
        "QProgressBar::chunk { background: #8B5CF6; }" :
        "QProgressBar { background: #E5E7EB; border: none; } "
        "QProgressBar::chunk { background: #8B5CF6; }";

    QList<QWidget*> widgets = findChildren<QWidget*>();
    for (QWidget* w : widgets) {
        if (qobject_cast<QProgressBar*>(w)) {
            w->setStyleSheet(barStyle);
        } else if (!qobject_cast<QLabel*>(w)) {
            w->setStyleSheet(baseStyle);
        }
    }
}