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
    , membershipLabel(nullptr)
    , classLabel(nullptr)
    , courtLabel(nullptr)
    , growthLabel(nullptr)
    , membershipBar(nullptr)
    , classBar(nullptr)
    , courtBar(nullptr)
    , growthBar(nullptr)
    , padelManager(nullptr)
{
    setupUI();
    // Call updateData after UI is set up
    QTimer::singleShot(0, this, &Revenue::updateData);
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
        bar->setRange(0, 100);  // Ensure range is set
        bar->setValue(0);       // Set initial value
        
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

void Revenue::updateData()
{
    // Check all required pointers
    if (!memberManager || !classManager || !userManager || 
        !membershipLabel || !classLabel || !courtLabel || !growthLabel ||
        !membershipBar || !classBar || !courtBar || !growthBar) {
        qDebug() << "Required pointers not initialized in updateData()";
        return;
    }

    try {
        QDate currentDate = timeLogicInstance.getCurrentTime().date();
        QDate monthStart(currentDate.year(), currentDate.month(), 1);
        QDate previousMonthStart = monthStart.addMonths(-1);

        // Get reports
        QVector<MonthlyReport> currentMonthReports = classManager->getMonthlyReports(monthStart, currentDate);
        MonthlyReport currentReport;

        // Generate report if none exists
        if (currentMonthReports.isEmpty()) {
            currentReport = classManager->generateMonthlyReport(monthStart);
        } else {
            currentReport = currentMonthReports.first();
        }

        // Get member data
        QVector<User> allUsers = userManager->getAllUsers();
        int totalMembers = 0, vipMembers = 0;
        
        for (const User& user : allUsers) {
            if (user.getId() > 0 && memberManager->userIsMember(user.getId())) {
                totalMembers++;
                if (memberManager->isVIPMember(user.getId())) {
                    vipMembers++;
                }
            }
        }

        // Update UI with validated data
        int vipPercentage = totalMembers > 0 ? (vipMembers * 100) / totalMembers : 0;
        updateMetrics(membershipLabel, membershipBar,
            QString("Members: %1\nVIP: %2%").arg(totalMembers).arg(vipPercentage),
            qBound(0, vipPercentage, 100));

        int attendanceRate = currentReport.totalClassesHeld > 0 ? 
            (currentReport.totalAttendance * 100) / (currentReport.totalClassesHeld * 30) : 0;
        updateMetrics(classLabel, classBar,
            QString("Classes: %1\nRevenue: $%2")
                .arg(currentReport.totalClassesHeld)
                .arg(currentReport.totalRevenue, 0, 'f', 2),
            qBound(0, attendanceRate, 100));

        int activeRate = totalMembers > 0 ? 
            (currentReport.totalActiveMembers * 100) / totalMembers : 0;
        updateMetrics(courtLabel, courtBar,
            QString("Active Members: %1\nRevenue/Member: $%2")
                .arg(currentReport.totalActiveMembers)
                .arg(totalMembers > 0 ? currentReport.totalRevenue / totalMembers : 0, 0, 'f', 2),
            qBound(0, activeRate, 100));

        // Calculate growth
        QVector<MonthlyReport> previousMonthReports = classManager->getMonthlyReports(
            previousMonthStart, monthStart.addDays(-1));
        
        double revenueGrowth = 0;
        int memberGrowth = 0;
        
        if (!previousMonthReports.isEmpty()) {
            const MonthlyReport& prevReport = previousMonthReports.first();
            if (prevReport.totalRevenue > 0) {
                revenueGrowth = ((currentReport.totalRevenue - prevReport.totalRevenue) * 100) 
                               / prevReport.totalRevenue;
            }
            if (prevReport.totalActiveMembers > 0) {
                memberGrowth = ((currentReport.totalActiveMembers - prevReport.totalActiveMembers) * 100) 
                              / prevReport.totalActiveMembers;
            }
        }

        updateMetrics(growthLabel, growthBar,
            QString("Revenue Growth: %1%\nMember Growth: %2%")
                .arg(revenueGrowth, 0, 'f', 1)
                .arg(memberGrowth),
            qBound(0, static_cast<int>(revenueGrowth), 100));


        //PADEL COURTS DATA
        if (padelManager) {
            int totalCourts = padelManager->getAllCourts().size();
            int bookedCourts = padelManager->getBookedCourtsCount();
            int courtUtilization = totalCourts > 0 ? (bookedCourts * 100) / totalCourts : 0;
            
            // Calculate VIP booking rate
            int totalBookings = 0;
            int vipBookings = 0;
            QVector<Booking> allBookings = padelManager->getAllBookings();
            for (const Booking& booking : allBookings) {
                if (!booking.isCancelled()) {
                    totalBookings++;
                    if (booking.isVip()) {
                        vipBookings++;
                    }
                }
            }
            int vipBookingRate = totalBookings > 0 ? (vipBookings * 100) / totalBookings : 0;
            
            updateMetrics(courtLabel, courtBar,
                QString("Court Utilization: %1%\nVIP Bookings: %2%")
                    .arg(courtUtilization)
                    .arg(vipBookingRate),
                courtUtilization);
        }

    } catch (const std::exception& e) {
        qDebug() << "Error updating revenue data:" << e.what();
    }
}

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