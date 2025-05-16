#ifndef REVENUE_H
#define REVENUE_H

#include <QWidget>
class QLabel;
class QProgressBar;
class UserDataManager;
class MemberDataManager;
class ClassDataManager;
class PadelDataManager;

class Revenue : public QWidget
{
    Q_OBJECT

public:
    explicit Revenue(UserDataManager* userDataManager, MemberDataManager* memberDataManager,
                    ClassDataManager* classDataManager, QWidget* parent = nullptr);
    void updateTheme(bool isDark);
    void updateData();

    // Data manager setters
    void setClassDataManager(ClassDataManager* manager) { classManager = manager; }
    void setUserDataManager(UserDataManager* manager) { userManager = manager; }
    void setMemberDataManager(MemberDataManager* manager) { memberManager = manager; }
    void setPadelDataManager(PadelDataManager* manager) { padelManager = manager; }

    // Data setters for testing/manual updates
    void setMembershipData(int vipPercentage, int regularPercentage);
    void setClassData(int totalClasses, int avgAttendance);
    void setCourtData(int utilizationRate, int vipBookingRate);
    void setGrowthData(int revenueGrowth, int memberGrowth);

private:
    void updateMetrics(QLabel* label, QProgressBar* bar, const QString& text, int value);
    void setupUI();

    bool isDarkTheme;
    UserDataManager* userManager;
    MemberDataManager* memberManager;
    ClassDataManager* classManager;
    PadelDataManager* padelManager;

    // UI Elements
    QLabel* membershipLabel;
    QLabel* classLabel;
    QLabel* courtLabel;
    QLabel* growthLabel;
    QProgressBar* membershipBar;
    QProgressBar* classBar;
    QProgressBar* courtBar;
    QProgressBar* growthBar;

    // Analytics data structure
    struct {
        int vipMemberPercentage{0};
        int regularMemberPercentage{0};
        int totalClasses{0};
        int averageAttendance{0};
        int courtUtilization{0};
        int vipBookingRate{0};
        int revenueGrowth{0};
        int memberGrowth{0};
    } analytics;
};

#endif // REVENUE_H 