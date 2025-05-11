#include "../Gym/member.h"
#include <QDebug>

// Constructor with member-specific data
Member::Member(int memberId, int userId, int classId)
    : memberId(memberId)
    , userId(userId)
    , classId(classId)
{
    // Initialize with default subscription
    subscription = Subscription(SubscriptionType::MONTHLY, QDate::currentDate());
  
}

Member::Member()
    : memberId(0)
    , userId(0)
    , classId(-1)
{
    // Initialize with default subscription
    subscription = Subscription(SubscriptionType::MONTHLY, QDate::currentDate());
}

void Member::addClassToHistory(const QDate& date) {
    history.append(qMakePair(classId, date));
}

QString Member::toString() const {
    return QString("Member[id=%1, userId=%2, classId=%3, hasActiveSubscription=%4]")
        .arg(memberId)
        .arg(userId)
        .arg(classId)
        .arg(hasActiveSubscription() ? "true" : "false");
}

void Member::getRemindersSubEnd() {
    if (!hasActiveSubscription()) {
        NotificationManager::instance().showNotification("Renew your subscription!", "Your subscription has ended click here to renew it.",
            [this]() {
              hadlesubpage();
            }, NotificationType::Info);
    }
}


void Member::hadlesubpage() {
    try {
        qDebug() << "Subscription::SubscriptionPage called";

        if (!stackedWidget) {
            qDebug() << "Error: stackedWidget is null in handlesubscriptionpage";
            return;
        }

        if (!subscriptionpage) {
            qDebug() << "Error: subscriptionpage is null in handleHomePage";
            return;
        }

        qDebug() << "Setting current widget to subscriptionpage";
        stackedWidget->setCurrentWidget(subscriptionpage);

     
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in Memper::subscriptionpage: " << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in MainPage::handleHomePage";
    }
}