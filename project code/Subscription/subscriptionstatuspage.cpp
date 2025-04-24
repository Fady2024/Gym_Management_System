#include "subscriptionstatuspage.h"
#include <QDebug>
#include <QPixmap>
#include <QDateTime>
#include <QIcon>
#include <QFrame>
#include <QScrollArea>
#include <QSizePolicy>
#include <QRegularExpression>
#include <QTimer>

SubscriptionStatusPage::SubscriptionStatusPage(QWidget *parent, MemberDataManager* memberManager, UserDataManager* userManager)
    : QWidget(parent)
    , memberManager(memberManager)
    , userManager(userManager)
    , currentMemberId(0)
    , isDarkTheme(false)
    , hasActiveSubscription(false)
    , isExpiringSoon(false)
    , daysRemaining(0)
    , currentPlan(SubscriptionType::MONTHLY)
    , isVipMember(false)
    , newUserView(nullptr)
{
    setupUI();
    
    // Default to existing member view (index 1) until we determine otherwise
    mainStack->setCurrentIndex(1);
    
    // Auto-load member data if available
    QTimer::singleShot(300, this, [this]() {
        qDebug() << "Auto-loading member data...";
        if (this->memberManager) {
            QVector<Member> allMembers = this->memberManager->getAllMembers();
            if (!allMembers.isEmpty()) {
                Member firstMember = allMembers.first();
                setCurrentMemberId(firstMember.getId());
                qDebug() << "Auto-loaded member ID:" << firstMember.getId();
            } else {
                qDebug() << "No members found during auto-load";
                // If no members, switch to the new user view
                mainStack->setCurrentIndex(0);
                if (newUserView) {
                    newUserView->updateTheme(isDarkTheme);
                    newUserView->updateLayout();
                }
            }
        } else {
            qDebug() << "Member manager is null during auto-load";
            // If no member manager, switch to the new user view
            mainStack->setCurrentIndex(0);
            if (newUserView) {
                newUserView->updateTheme(isDarkTheme);
                newUserView->updateLayout();
            }
        }
    });
}

SubscriptionStatusPage::~SubscriptionStatusPage()
{
    // No need for explicit cleanup as Qt will handle deleting child widgets
}

void SubscriptionStatusPage::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Create a stacked widget to switch between views
    mainStack = new QStackedWidget(this);
    mainStack->setObjectName("mainStack");
    
    // Create new user view
    newUserView = new NewUserSubscriptionView(this);
    connect(newUserView, &NewUserSubscriptionView::subscribeRequested, this, &SubscriptionStatusPage::subscribeRequested);
    mainStack->addWidget(newUserView);

    // Create content for existing users
    QWidget* existingUserPage = new QWidget(this);
    QVBoxLayout* existingUserLayout = new QVBoxLayout(existingUserPage);
    existingUserLayout->setSpacing(0);
    existingUserLayout->setContentsMargins(40, 40, 40, 40);
    existingUserLayout->setAlignment(Qt::AlignCenter);

    // Create a scroll area for responsive design
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Style the scroll bar for a more modern look
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
        }
        QScrollBar:vertical {
            border: none;
            background: rgba(255, 255, 255, 0.05);
            width: 8px;
            border-radius: 4px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: rgba(139, 92, 246, 0.6);
            min-height: 30px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(139, 92, 246, 0.8);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

    contentContainer = new QWidget;
    contentContainer->setObjectName("contentContainer");
    contentContainer->setStyleSheet("QWidget#contentContainer { background: transparent; }");
    contentContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* containerLayout = new QVBoxLayout(contentContainer);
    containerLayout->setSpacing(32);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // Title and header section
    titleLabel = new QLabel(tr("Subscription Status"));
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);

    // Create status card with elevated design
    createStatusCard();
    containerLayout->addWidget(statusCard, 0, Qt::AlignHCenter);

    // Create action buttons (including Renew)
    createActionButtons();
    containerLayout->addWidget(actionsContainer, 0, Qt::AlignHCenter);

    // Add bottom spacing
    containerLayout->addStretch(1);

    // Set the content container as the scroll area's widget
    scrollArea->setWidget(contentContainer);
    
    // Add the scroll area to the existing user layout
    existingUserLayout->addWidget(scrollArea);
    
    // Add existing user page to stack
    mainStack->addWidget(existingUserPage);
    
    // Add the stack to the main layout
    mainLayout->addWidget(mainStack);
    
    // Apply initial styling
    setupCardStyles();
}

void SubscriptionStatusPage::createStatusCard()
{
    statusCard = new QWidget;
    statusCard->setObjectName("statusCard");
    statusCard->setMinimumWidth(580);
    statusCard->setMaximumWidth(800);
    statusCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* cardLayout = new QVBoxLayout(statusCard);
    cardLayout->setSpacing(20);
    cardLayout->setContentsMargins(36, 36, 36, 36);

    // Greeting and member info
    greetingLabel = new QLabel(tr("Welcome back!"));
    greetingLabel->setObjectName("greetingLabel");
    cardLayout->addWidget(greetingLabel);

    memberInfoLabel = new QLabel();
    memberInfoLabel->setObjectName("memberInfoLabel");
    cardLayout->addWidget(memberInfoLabel);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setObjectName("divider");
    cardLayout->addWidget(divider);

    // Status header with icon
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(16);
    
    statusIconLabel = new QLabel();
    statusIconLabel->setObjectName("statusIconLabel");
    statusIconLabel->setFixedSize(32, 32);
    statusLayout->addWidget(statusIconLabel);
    
    statusTextLabel = new QLabel();
    statusTextLabel->setObjectName("statusTextLabel");
    statusLayout->addWidget(statusTextLabel);
    
    statusLayout->addStretch();
    cardLayout->addLayout(statusLayout);

    // Plan type
    planTypeLabel = new QLabel();
    planTypeLabel->setObjectName("planTypeLabel");
    planTypeLabel->setWordWrap(true);
    cardLayout->addWidget(planTypeLabel);

    // Dates
    datesLabel = new QLabel();
    datesLabel->setObjectName("datesLabel");
    datesLabel->setWordWrap(true);
    cardLayout->addWidget(datesLabel);

    // Price
    priceLabel = new QLabel();
    priceLabel->setObjectName("priceLabel");
    cardLayout->addWidget(priceLabel);

    // Days remaining progress
    QWidget* progressContainer = new QWidget();
    QVBoxLayout* progressLayout = new QVBoxLayout(progressContainer);
    progressLayout->setSpacing(12);
    progressLayout->setContentsMargins(0, 16, 0, 0);

    remainingDaysLabel = new QLabel();
    remainingDaysLabel->setObjectName("remainingDaysLabel");
    progressLayout->addWidget(remainingDaysLabel);

    daysProgressBar = new QProgressBar();
    daysProgressBar->setObjectName("daysProgressBar");
    daysProgressBar->setTextVisible(false);
    daysProgressBar->setFixedHeight(10);
    daysProgressBar->setRange(0, 100);
    progressLayout->addWidget(daysProgressBar);

    cardLayout->addWidget(progressContainer);

    // History message (optional)
    historyLabel = new QLabel();
    historyLabel->setObjectName("historyLabel");
    historyLabel->setWordWrap(true);
    cardLayout->addWidget(historyLabel);
}

void SubscriptionStatusPage::createActionButtons()
{
    actionsContainer = new QWidget();
    actionsContainer->setObjectName("actionsContainer");
    actionsContainer->setMinimumWidth(560);
    actionsContainer->setMaximumWidth(800);

    QHBoxLayout* buttonsLayout = new QHBoxLayout(actionsContainer);
    buttonsLayout->setSpacing(16);
    buttonsLayout->setContentsMargins(0, 16, 0, 0);

    // Subscribe button - for new users
    subscribeButton = new QPushButton(tr("Subscribe Now"));
    subscribeButton->setObjectName("subscribeButton");
    subscribeButton->setFixedHeight(48);
    subscribeButton->setCursor(Qt::PointingHandCursor);
    connect(subscribeButton, &QPushButton::clicked, this, &SubscriptionStatusPage::subscribeRequested);

    // Renew button - for renewals
    renewButton = new QPushButton(tr("Renew Subscription"));
    renewButton->setObjectName("renewButton");
    renewButton->setFixedHeight(48);
    renewButton->setCursor(Qt::PointingHandCursor);
    connect(renewButton, &QPushButton::clicked, this, [this]() {
        emit renewRequested(static_cast<int>(currentPlan), isVipMember);
    });

    // Change plan button
    changePlanButton = new QPushButton(tr("Change Plan"));
    changePlanButton->setObjectName("changePlanButton");
    changePlanButton->setFixedHeight(48);
    changePlanButton->setCursor(Qt::PointingHandCursor);
    connect(changePlanButton, &QPushButton::clicked, this, &SubscriptionStatusPage::changePlanRequested);

    // Add buttons to layout
    buttonsLayout->addWidget(subscribeButton);
    buttonsLayout->addWidget(renewButton);
    buttonsLayout->addWidget(changePlanButton);

    // Initially hide all buttons - they'll be shown based on status
    subscribeButton->hide();
    renewButton->hide();
    changePlanButton->hide();
}

void SubscriptionStatusPage::setupCardStyles()
{
    // Card styling with more elegant gradient background and enhanced depth
    QString cardBackground = isDarkTheme ? 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(31, 41, 55, 0.98), stop:0.5 rgba(17, 24, 39, 0.98), stop:1 rgba(15, 23, 42, 0.98))" : 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0.98), stop:0.5 rgba(249, 250, 251, 0.98), stop:1 rgba(243, 244, 246, 0.98))";
    
    QString borderColor = isDarkTheme ? 
        "rgba(74, 85, 104, 0.4)" : 
        "rgba(226, 232, 240, 0.9)";
    
    statusCard->setStyleSheet(QString(R"(
        QWidget#statusCard {
            background: %1;
            border-radius: 24px;
            border: 1px solid %2;
        }
    )").arg(cardBackground, borderColor));

    // Stronger shadow for more depth with proper color based on theme
    QGraphicsDropShadowEffect* cardShadow = new QGraphicsDropShadowEffect(statusCard);
    cardShadow->setBlurRadius(36);
    cardShadow->setColor(QColor(0, 0, 0, isDarkTheme ? 100 : 50));
    cardShadow->setOffset(0, 8);
    statusCard->setGraphicsEffect(cardShadow);

    // Title styling with more dramatic text shadow and better typography
    titleLabel->setStyleSheet(QString(R"(
        QLabel#titleLabel {
            color: %1;
            font-size: 36px;
            font-weight: 800;
            letter-spacing: -0.5px;
            margin-bottom: 32px;
            %2
        }
    )").arg(
        isDarkTheme ? "#F9FAFB" : "#1F2937",
        isDarkTheme ? "text-shadow: 0px 2px 4px rgba(0, 0, 0, 0.3);" : "text-shadow: 0px 1px 2px rgba(0, 0, 0, 0.15);"
    ));

    // Greeting styling with enhanced typography
    greetingLabel->setStyleSheet(QString(R"(
        QLabel#greetingLabel {
            color: %1;
            font-size: 28px;
            font-weight: 700;
            letter-spacing: -0.5px;
        }
    )").arg(isDarkTheme ? "#F9FAFB" : "#1F2937"));

    // Member info styling
    memberInfoLabel->setStyleSheet(QString(R"(
        QLabel#memberInfoLabel {
            color: %1;
            font-size: 16px;
            margin-bottom: 12px;
            font-weight: 500;
        }
    )").arg(isDarkTheme ? "#9CA3AF" : "#6B7280"));

    // Divider styling with gradient
    QString dividerColor = isDarkTheme ? 
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(74, 85, 104, 0.1), stop:0.5 rgba(74, 85, 104, 0.7), stop:1 rgba(74, 85, 104, 0.1))" : 
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(226, 232, 240, 0.1), stop:0.5 rgba(226, 232, 240, 0.9), stop:1 rgba(226, 232, 240, 0.1))";
    
    statusCard->findChild<QFrame*>("divider")->setStyleSheet(QString(R"(
        QFrame#divider {
            background: %1;
            margin: 20px 0px;
            border: none;
            height: 1px;
        }
    )").arg(dividerColor));

    // Status text styling - will be dynamically updated based on status
    statusTextLabel->setStyleSheet(QString(R"(
        QLabel#statusTextLabel {
            font-size: 22px;
            font-weight: 700;
            letter-spacing: -0.3px;
        }
    )"));

    // Labels styling with increased size and better contrast
    QString labelStyle = QString(R"(
        QLabel {
            color: %1;
            font-size: 18px;
            font-weight: 500;
            margin-top: 8px;
            letter-spacing: 0.1px;
        }
    )").arg(isDarkTheme ? "#E5E7EB" : "#374151");

    planTypeLabel->setStyleSheet(labelStyle);
    datesLabel->setStyleSheet(labelStyle);
    priceLabel->setStyleSheet(labelStyle);
    historyLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 16px;
            font-style: italic;
            margin-top: 24px;
            line-height: 1.5;
        }
    )").arg(isDarkTheme ? "#9CA3AF" : "#6B7280"));

    // Progress bar styling with improved radius and animation
    daysProgressBar->setFixedHeight(12);
    daysProgressBar->setStyleSheet(QString(R"(
        QProgressBar {
            background-color: %1;
            border-radius: 6px;
            border: none;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: %2;
            border-radius: 6px;
        }
    )").arg(
        isDarkTheme ? "rgba(75, 85, 99, 0.3)" : "rgba(229, 231, 235, 0.8)",
        hasActiveSubscription ? (isExpiringSoon ? "#F59E0B" : "#10B981") : "#EF4444"
    ));

    // Remaining days label styling with enhanced visual appeal
    remainingDaysLabel->setStyleSheet(QString(R"(
        QLabel#remainingDaysLabel {
            color: %1;
            font-size: 18px;
            font-weight: 600;
            letter-spacing: 0.2px;
            margin-bottom: 8px;
        }
    )").arg(
        hasActiveSubscription ? (isExpiringSoon ? "#F59E0B" : "#10B981") : "#EF4444"
    ));

    // Button styling with more modern look
    QString baseButtonStyle = QString(R"(
        QPushButton {
            border-radius: 16px;
            font-size: 18px;
            font-weight: 600;
            padding: 14px 28px;
            min-width: 200px;
            transition: all 0.3s;
            letter-spacing: 0.2px;
        }
        QPushButton:hover {
            transform: translateY(-2px);
            opacity: 0.95;
        }
    )");

    // Subscribe button - purple gradient with enhanced visual appeal
    subscribeButton->setStyleSheet(baseButtonStyle + QString(R"(
        QPushButton#subscribeButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:0.5 #7C3AED, stop:1 #6D28D9);
            color: white;
            border: none;
        }
        QPushButton#subscribeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7C3AED, stop:0.5 #6D28D9, stop:1 #5B21B6);
        }
    )"));

    // Renew button - more vibrant gradient with gold/amber tones
    renewButton->setStyleSheet(baseButtonStyle + QString(R"(
        QPushButton#renewButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F59E0B, stop:0.5 #FBBF24, stop:1 #F59E0B);
            color: white;
            font-weight: 700;
            border: none;
        }
        QPushButton#renewButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D97706, stop:0.5 #F59E0B, stop:1 #D97706);
        }
    )"));

    // Change plan button - more subtle but with glass-like effect
    changePlanButton->setStyleSheet(baseButtonStyle + QString(R"(
        QPushButton#changePlanButton {
            background-color: %1;
            color: %2;
            border: 1px solid %3;
            backdrop-filter: blur(10px);
        }
        QPushButton#changePlanButton:hover {
            background-color: %4;
            border-color: %5;
        }
    )").arg(
        isDarkTheme ? "rgba(55, 65, 81, 0.7)" : "rgba(249, 250, 251, 0.8)",
        isDarkTheme ? "#F9FAFB" : "#1F2937",
        isDarkTheme ? "rgba(75, 85, 99, 0.7)" : "rgba(209, 213, 219, 0.9)",
        isDarkTheme ? "rgba(55, 65, 81, 0.9)" : "rgba(243, 244, 246, 0.9)",
        isDarkTheme ? "rgba(107, 114, 128, 0.8)" : "rgba(156, 163, 175, 0.9)"
    ));

    // Add enhanced shadows to buttons for more depth
    QGraphicsDropShadowEffect* subscribeButtonShadow = new QGraphicsDropShadowEffect(subscribeButton);
    subscribeButtonShadow->setBlurRadius(20);
    subscribeButtonShadow->setColor(QColor(139, 92, 246, 100));
    subscribeButtonShadow->setOffset(0, 5);
    subscribeButton->setGraphicsEffect(subscribeButtonShadow);

    QGraphicsDropShadowEffect* renewButtonShadow = new QGraphicsDropShadowEffect(renewButton);
    renewButtonShadow->setBlurRadius(20);
    renewButtonShadow->setColor(QColor(245, 158, 11, 100));
    renewButtonShadow->setOffset(0, 5);
    renewButton->setGraphicsEffect(renewButtonShadow);

    QGraphicsDropShadowEffect* changePlanButtonShadow = new QGraphicsDropShadowEffect(changePlanButton);
    changePlanButtonShadow->setBlurRadius(14);
    changePlanButtonShadow->setColor(QColor(0, 0, 0, 50));
    changePlanButtonShadow->setOffset(0, 3);
    changePlanButton->setGraphicsEffect(changePlanButtonShadow);
}

void SubscriptionStatusPage::setupRegularView()
{
    // Show all standard elements for subscribers
    greetingLabel->setVisible(true);
    memberInfoLabel->setVisible(true);
    statusTextLabel->setVisible(true);
    statusIconLabel->setVisible(true);
    planTypeLabel->setVisible(true);
    datesLabel->setVisible(true);
    priceLabel->setVisible(true);
    daysProgressBar->setVisible(true);
    remainingDaysLabel->setVisible(true);
    
    // Restore standard styling
    planTypeLabel->setStyleSheet("");
    historyLabel->setStyleSheet("");
    
    // Set visibility of action buttons
    subscribeButton->setVisible(false);
    renewButton->setVisible(true); // Always show renew button for active subscriptions
    changePlanButton->setVisible(hasActiveSubscription);
    
    // Switch to existing user view (index 1 in stack)
    mainStack->setCurrentIndex(1);
    
    // Add debug information
    qDebug() << "Set up regular view with renewButton visible:" << renewButton->isVisible();
    qDebug() << "hasActiveSubscription:" << hasActiveSubscription;
    qDebug() << "isExpiringSoon:" << isExpiringSoon;
}

void SubscriptionStatusPage::loadMemberData()
{
    qDebug() << "SubscriptionStatusPage::loadMemberData() called, currentMemberId:" << currentMemberId;
    
    // First check if we have valid member manager
    if (!memberManager) {
        qDebug() << "Member manager is null, showing new user view";
        // No member manager, show new user view
        mainStack->setCurrentIndex(0);
        newUserView->updateTheme(isDarkTheme);
        newUserView->updateLayout();
        return;
    }
    
    // If currentMemberId is 0, we should show the new user view instead of loading the first member
    if (currentMemberId <= 0) {
        qDebug() << "No valid member ID provided, showing new user view";
        mainStack->setCurrentIndex(0);
        newUserView->updateTheme(isDarkTheme);
        newUserView->updateLayout();
        return;
    }

    // Get member data
    Member member = memberManager->getMemberById(currentMemberId);
    if (member.getId() <= 0) {
        qDebug() << "Member ID not found:" << currentMemberId;
        // Show new user view (index 0 in stack)
        mainStack->setCurrentIndex(0);
        // Update theme on the new user view
        newUserView->updateTheme(isDarkTheme);
        // Update layout
        newUserView->updateLayout();
        return;
    }

    qDebug() << "Member found with ID:" << member.getId() << ", loading subscription data";

    // Member exists, check subscription status
    const Subscription& subscription = member.getSubscription();
    hasActiveSubscription = subscription.isActive();
    
    // Get member name from user data
    memberName = "";
    if (userManager) {
        User user = userManager->getUserDataById(member.getUserId());
        if (user.getId() > 0) {
            memberName = user.getName().split(" ").first(); // Get first name
            qDebug() << "Retrieved member name: " << memberName << " from user ID: " << user.getId();
        } else {
            qDebug() << "Failed to retrieve user data for user ID: " << member.getUserId();
        }
    } else {
        qDebug() << "User manager is null, cannot retrieve member name";
    }
    
    if (!hasActiveSubscription) {
        qDebug() << "No active subscription found, showing returning member promo view";
        // Member with expired subscription - show returning member promo
        endDate = subscription.getEndDate();
        currentPlan = subscription.getType();
        setupReturningMemberPromoView();
        
        // Make sure we're showing the existing user view (index 1)
        mainStack->setCurrentIndex(1);
        return;
    }

    // Active subscription - show regular view
    isVipMember = subscription.isVIP();
    currentPlan = subscription.getType();
    startDate = subscription.getStartDate();
    endDate = subscription.getEndDate();
    daysRemaining = QDate::currentDate().daysTo(endDate);
    isExpiringSoon = daysRemaining > 0 && daysRemaining <= 30;

    qDebug() << "Loaded active subscription: Days remaining:" << daysRemaining
             << ", Start date:" << startDate.toString()
             << ", End date:" << endDate.toString()
             << ", Is VIP:" << (isVipMember ? "Yes" : "No")
             << ", Is expiring soon:" << (isExpiringSoon ? "Yes" : "No");

    setupRegularView();
    updateStatusCard();
    updateMemberInfo();
    updateStatusColors();
    updateRemainingDays();
    
    // Make sure we're showing the existing user view (index 1)
    mainStack->setCurrentIndex(1);
    
    qDebug() << "Current stack index:" << mainStack->currentIndex()
             << ", subscribeButton visible:" << subscribeButton->isVisible()
             << ", renewButton visible:" << renewButton->isVisible()
             << ", changePlanButton visible:" << changePlanButton->isVisible();
}

void SubscriptionStatusPage::updateStatusCard()
{
    if (hasActiveSubscription) {
        // Active subscription
        statusTextLabel->setText(tr("Your subscription is active"));
        QPixmap activeIcon(":/Images/check.png");
        if (!activeIcon.isNull()) {
            statusIconLabel->setPixmap(activeIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        
        QDate currentDate = QDate::currentDate();
        int daysUntilExpiry = currentDate.daysTo(endDate);
        
        // Check if expiring soon (within 30 days)
        isExpiringSoon = daysUntilExpiry <= 30 && daysUntilExpiry > 0;
        
        if (isExpiringSoon) {
            statusTextLabel->setText(tr("Your subscription expires in %1 days").arg(daysUntilExpiry));
            renewButton->setVisible(true);
            subscribeButton->setVisible(false);
        } else {
            // Subscription active and not expiring soon
            renewButton->setVisible(true);
            subscribeButton->setVisible(false);
        }
        
        // Always allow changing plan for active subscriptions
        changePlanButton->setVisible(true);
        
        // Update plan details
        planTypeLabel->setText(tr("Plan: %1%2").arg(
            getSubscriptionTypeName(currentPlan),
            isVipMember ? tr(" (VIP)") : ""
        ));
        
        datesLabel->setText(tr("Valid from %1 to %2").arg(
            startDate.toString("MMMM d, yyyy"),
            endDate.toString("MMMM d, yyyy")
        ));
        
        priceLabel->setText(tr("Subscription Price: $%1").arg(
            QString::number(getSubscriptionPrice(currentPlan, isVipMember), 'f', 2)
        ));
        
        // Make all related information visible
        daysProgressBar->setVisible(true);
        remainingDaysLabel->setVisible(true);
        planTypeLabel->setVisible(true);
        datesLabel->setVisible(true);
        priceLabel->setVisible(true);
    } else {
        // No active subscription
        statusTextLabel->setText(tr("You don't have an active subscription"));
        QPixmap inactiveIcon(":/Images/error.png");
        if (!inactiveIcon.isNull()) {
            statusIconLabel->setPixmap(inactiveIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        
        // Show subscribe button only
        subscribeButton->setVisible(true);
        renewButton->setVisible(false);
        changePlanButton->setVisible(false);
        
        // Hide subscription details
        daysProgressBar->setVisible(false);
        remainingDaysLabel->setVisible(false);
        planTypeLabel->setVisible(false);
        datesLabel->setVisible(false);
        priceLabel->setVisible(false);
    }
    
    // Update colors and styling
    updateRemainingDays();
}

void SubscriptionStatusPage::updateMemberInfo()
{
    // Greeting with member name
    if (!memberName.isEmpty()) {
        greetingLabel->setText(tr("Welcome back, %1!").arg(memberName));
    } else {
        greetingLabel->setText(tr("Welcome!"));
    }
    
    // Member ID and subtitle
    memberInfoLabel->setText(tr("Member ID: %1 | Let's check your subscription status").arg(currentMemberId));
}

void SubscriptionStatusPage::updateStatusColors()
{
    QString statusText;
    QString statusColor;
    QPixmap statusIcon;
    
    if (!hasActiveSubscription) {
        statusText = tr("No active subscription");
        statusColor = "#EF4444"; // Red
        statusIcon = QPixmap(":/Images/error.png");
    } else if (isExpiringSoon) {
        statusText = tr("Your subscription is expiring soon");
        statusColor = "#F59E0B"; // Amber
        statusIcon = QPixmap(":/Images/warning.png");
    } else {
        statusText = isVipMember ? 
            tr("Your subscription is active (VIP)") : 
            tr("Your subscription is active");
        statusColor = "#10B981"; // Green
        statusIcon = QPixmap(":/Images/check.png");
    }
    
    // Set status text and color
    statusTextLabel->setText(statusText);
    statusTextLabel->setStyleSheet(QString("QLabel#statusTextLabel { color: %1; font-size: 18px; font-weight: 600; }").arg(statusColor));
    
    // Set icon if available
    if (!statusIcon.isNull()) {
        statusIconLabel->setPixmap(statusIcon.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    // Enhanced progress bar with better gradient and animation
    if (hasActiveSubscription) {
        QString progressColor = isExpiringSoon ? "#F59E0B" : "#10B981";
        QString bgColor = isDarkTheme ? "rgba(74, 85, 104, 0.3)" : "rgba(209, 213, 219, 0.8)";
        
    daysProgressBar->setStyleSheet(QString(R"(
        QProgressBar {
            background-color: %1;
                border-radius: 6px;
            border: none;
                text-align: center;
                height: 12px;
        }
        QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 rgba(255, 255, 255, 0.3));
                border-radius: 6px;
            }
        )").arg(bgColor, progressColor));
    } else {
        daysProgressBar->setStyleSheet(QString(R"(
            QProgressBar {
                background-color: %1;
                border-radius: 6px;
                border: none;
                height: 12px;
            }
            QProgressBar::chunk {
                background-color: #EF4444;
                border-radius: 6px;
            }
        )").arg(isDarkTheme ? "rgba(74, 85, 104, 0.3)" : "rgba(209, 213, 219, 0.8)"));
    }
    
    // Update plan details with better text contrast
    QString textColor = isDarkTheme ? "#E5E7EB" : "#374151";
    
    planTypeLabel->setText(tr("Plan: %1%2").arg(
        getSubscriptionTypeName(currentPlan),
        isVipMember ? tr(" (VIP)") : ""
    ));
    planTypeLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 18px; font-weight: 500; }").arg(textColor));
    
    datesLabel->setText(tr("Valid from %1 to %2").arg(
        startDate.toString("MMMM d, yyyy"),
        endDate.toString("MMMM d, yyyy")
    ));
    datesLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 18px; font-weight: 500; }").arg(textColor));
    
    // Price label with slightly different styling to make it stand out
    priceLabel->setText(tr("Subscription Price: $%1").arg(
        QString::number(getSubscriptionPrice(currentPlan, isVipMember), 'f', 2)
    ));
    priceLabel->setStyleSheet(QString("QLabel { color: %1; font-size: 18px; font-weight: 600; }").arg(textColor));
}

void SubscriptionStatusPage::updateRemainingDays()
{
    // Calculate remaining days
    QDate currentDate = QDate::currentDate();
    
    // Ensure we have valid dates
    if (!startDate.isValid() || !endDate.isValid()) {
        remainingDaysLabel->setText(tr("Date information not available"));
        daysProgressBar->setValue(0);
        daysProgressBar->setStyleSheet(getProgressBarStyle(0));
        return;
    }
    
    int totalDays = startDate.daysTo(endDate);
    int remainingDays = currentDate.daysTo(endDate);
    
    if (remainingDays < 0) {
        remainingDays = 0;
    }
    
    // Update the UI with more visually appealing formatted text showing days remaining
    if (remainingDays > 0) {
        // Set text color with better contrast for both themes
        QString daysColor = isExpiringSoon ? "#F59E0B" : "#10B981"; // Amber or Green
        
        // Make sure "days remaining" text is definitely visible in both themes with high contrast
        QString textColor = isDarkTheme ? "#FFFFFF" : "#111827"; // White in dark theme, very dark gray in light theme
        
        // Create the text with consistent styling and guaranteed visibility
        remainingDaysLabel->setText(tr("<span style='color:%1; font-size:20px; font-weight:600;'>%2</span> <span style='color:%3; font-weight:500; font-size:16px;'>days remaining</span>")
            .arg(daysColor)
            .arg(remainingDays)
            .arg(textColor));
    } else {
        // For expired subscription, use red color
        remainingDaysLabel->setText(tr("<span style='color:#EF4444; font-size:20px; font-weight:600;'>Subscription expired</span>"));
    }
    
    // Calculate progress percentage - how much of subscription has elapsed
    int elapsedDays = totalDays - remainingDays;
    int progressPercentage = (elapsedDays * 100) / (totalDays > 0 ? totalDays : 1);
    
    // Ensure progress percentage is valid
    progressPercentage = qMax(0, qMin(progressPercentage, 100));
    
    // Set the progress bar style and value
    daysProgressBar->setStyleSheet(getProgressBarStyle(progressPercentage));
    daysProgressBar->setValue(progressPercentage);
    
    // Make sure the progress components are visible
    daysProgressBar->setVisible(true);
    remainingDaysLabel->setVisible(true);
}

QString SubscriptionStatusPage::getProgressBarStyle(int progressPercentage) const
{
    // Define colors based on theme and progress with enhanced visuals
    // Use a darker background color for light theme to improve contrast
    QString backgroundColor = isDarkTheme ? 
                             "rgba(75, 85, 99, 0.3)" : 
                             "rgba(209, 213, 219, 0.8)";
    
    QString progressColorBase;
    QString progressColorEnd;
    
    // Create gradient colors based on subscription status
    if (hasActiveSubscription) {
        if (isExpiringSoon) {
            // Amber/orange gradient for expiring soon
            progressColorBase = "#F59E0B";
            progressColorEnd = "#FBBF24";
    } else {
            // Green gradient for active
            progressColorBase = "#10B981";
            progressColorEnd = "#34D399";
        }
    } else {
        // Red gradient for expired
        progressColorBase = "#EF4444";
        progressColorEnd = "#F87171";
    }
    
    return QString("QProgressBar {"
                  "background-color: %1;"
                  "border-radius: 6px;"
                  "text-align: center;"
                  "height: 12px;"
                  "min-height: 12px;"
                  "max-height: 12px;"
                  "margin: 10px 0px;"
                  "}"
                  "QProgressBar::chunk {"
                  "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 %2, stop:1 %3);"
                  "border-radius: 6px;"
                  "}")
            .arg(backgroundColor)
            .arg(progressColorBase)
            .arg(progressColorEnd);
}

// Helper method to get subscription type name
QString SubscriptionStatusPage::getSubscriptionTypeName(SubscriptionType type)
{
    switch (type) {
        case SubscriptionType::MONTHLY: return tr("Monthly");
        case SubscriptionType::THREE_MONTHS: return tr("Three Months");
        case SubscriptionType::SIX_MONTHS: return tr("Six Months");
        case SubscriptionType::YEARLY: return tr("Yearly");
        default: return tr("Unknown");
    }
}

// Helper method to get subscription price
double SubscriptionStatusPage::getSubscriptionPrice(SubscriptionType type, bool isVip)
{
    double basePrice = 0.0;
    
    switch (type) {
        case SubscriptionType::MONTHLY:
            basePrice = 29.99;
            break;
        case SubscriptionType::THREE_MONTHS:
            basePrice = 79.99;
            break;
        case SubscriptionType::SIX_MONTHS:
            basePrice = 149.99;
            break;
        case SubscriptionType::YEARLY:
            basePrice = 249.99;
            break;
    }
    
    if (isVip) {
        // Add VIP price based on type
        switch (type) {
            case SubscriptionType::MONTHLY:
                basePrice += 9.99;
                break;
            case SubscriptionType::THREE_MONTHS:
                basePrice += 24.99;
                break;
            case SubscriptionType::SIX_MONTHS:
                basePrice += 44.99;
                break;
            case SubscriptionType::YEARLY:
                basePrice += 74.99;
                break;
        }
    }
    
    return basePrice;
}

void SubscriptionStatusPage::setupReturningMemberPromoView()
{
    // Hide subscription-specific elements
    daysProgressBar->hide();
    remainingDaysLabel->hide();

    // Show personalized welcome back message
    if (!memberName.isEmpty()) {
        greetingLabel->setText(tr("Welcome back, %1!").arg(memberName));
    } else {
        greetingLabel->setText(tr("Welcome back!"));
    }
    memberInfoLabel->setText(tr("Member ID: %1 | We've missed you!").arg(currentMemberId));
    memberInfoLabel->show();

    // Status message for returning members
    statusTextLabel->setText(tr("No active subscription"));
    statusTextLabel->setStyleSheet("QLabel#statusTextLabel { color: #EF4444; font-size: 22px; font-weight: 700; }");
    statusTextLabel->show();

    // Set icon
    QPixmap statusIcon(":/Images/error.png");
    if (!statusIcon.isNull()) {
        statusIconLabel->setPixmap(statusIcon.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    statusIconLabel->show();

    // Show previous plan info and special offer
    planTypeLabel->setWordWrap(true);
    planTypeLabel->setText(tr("Your previous %1 plan gave you access to premium features. "
                            "Renew now to continue your fitness journey and unlock:"
                            "\n\n• Your saved workout history"
                            "\n• Personalized fitness plans"
                            "\n• Premium features and classes")
                            .arg(getSubscriptionTypeName(currentPlan)));
    planTypeLabel->show();

    // Show dates if available
    if (endDate.isValid()) {
        datesLabel->setText(tr("Previous subscription ended: %1")
            .arg(endDate.toString("MMMM d, yyyy")));
        datesLabel->show();
    } else {
        datesLabel->hide();
    }

    // Hide price label as it's not relevant for expired subscriptions
    priceLabel->hide();

    // Show special renewal offer
    historyLabel->setText(tr("Special offer: Renew now and get 15% off your first month!"));
    historyLabel->setStyleSheet(QString("QLabel#historyLabel { color: %1; font-size: 18px; font-weight: 600; margin-top: 20px; }")
        .arg(isDarkTheme ? "#F59E0B" : "#D97706"));
    historyLabel->show();

    // Configure buttons for returning members
    subscribeButton->hide();
    renewButton->setText(tr("Renew Now"));
    renewButton->setVisible(true);
    changePlanButton->setText(tr("Explore Plans"));
    changePlanButton->setVisible(true);

    // Apply special styling to the card to make it stand out
    QString cardBackground = isDarkTheme ? 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(30, 41, 59, 0.98), stop:1 rgba(15, 23, 42, 0.98))" : 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0.98), stop:1 rgba(248, 250, 252, 0.98))";
    
    statusCard->setStyleSheet(QString(R"(
        QWidget#statusCard {
            background: %1;
            border-radius: 20px;
            border: 1px solid %2;
        }
    )").arg(
        cardBackground,
        isDarkTheme ? "rgba(239, 68, 68, 0.4)" : "rgba(239, 68, 68, 0.2)"
    ));

    // Style the renew button
    renewButton->setStyleSheet(QString(R"(
        QPushButton#renewButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #F59E0B, stop:1 #FBBF24);
            color: white;
            border: none;
            border-radius: 14px;
            font-size: 18px;
            font-weight: 700;
            padding: 14px 32px;
            min-width: 200px;
        }
        QPushButton#renewButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #D97706, stop:1 #F59E0B);
        }
    )"));
}

void SubscriptionStatusPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Update new user view theme
    if (newUserView) {
        newUserView->updateTheme(isDark);
    }
    
    // Update scroll area theme
    QScrollArea* scrollArea = mainStack->widget(1)->findChild<QScrollArea*>();
    if (scrollArea) {
        scrollArea->setStyleSheet(QString(R"(
            QScrollArea {
                background: transparent;
                border: none;
            }
            QScrollBar:vertical {
                border: none;
                background: %1;
                width: 8px;
                border-radius: 4px;
                margin: 0px;
            }
            QScrollBar::handle:vertical {
                background: %2;
                min-height: 30px;
                border-radius: 4px;
            }
            QScrollBar::handle:vertical:hover {
                background: %3;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                background: none;
            }
        )").arg(
            isDark ? "rgba(255, 255, 255, 0.05)" : "rgba(0, 0, 0, 0.05)",
            isDark ? "rgba(139, 92, 246, 0.6)" : "rgba(139, 92, 246, 0.5)",
            isDark ? "rgba(139, 92, 246, 0.8)" : "rgba(139, 92, 246, 0.7)"
        ));
    }
    
    setupCardStyles();
    updateStatusColors(); // Re-apply status colors
    
    // Important: Update the remaining days label with the new theme color
    updateRemainingDays();
}

void SubscriptionStatusPage::setCurrentMemberId(int memberId)
{
    qDebug() << "SubscriptionStatusPage::setCurrentMemberId called with memberId:" << memberId 
             << ", current memberId:" << currentMemberId;
             
    // Reset state first, regardless of the ID
    hasActiveSubscription = false;
    isExpiringSoon = false;
    daysRemaining = 0;
    memberName = "";
    startDate = QDate();
    endDate = QDate();
    currentPlan = SubscriptionType::MONTHLY;
    isVipMember = false;
    
    // Update the member ID
    currentMemberId = memberId;
    
    // Only load member data if we have a positive ID
    if (currentMemberId > 0) {
        qDebug() << "Member ID updated to a positive value, calling loadMemberData()";
        loadMemberData();
    } else {
        qDebug() << "Member ID set to 0 or negative, showing new user view";
        // Reset UI to new user view
        if (mainStack) {
            mainStack->setCurrentIndex(0);
            if (newUserView) {
                newUserView->updateTheme(isDarkTheme);
                newUserView->updateLayout();
            }
        }
    }
}

void SubscriptionStatusPage::updateLayout()
{
    // Update layout based on current view
    if (mainStack->currentIndex() == 0) {
        // New user view
        newUserView->updateLayout();
    } else {
        // Existing user view
    // Ensure card and buttons have appropriate widths
    int cardWidth = qMin(800, qMax(560, width() - 80));
    statusCard->setFixedWidth(cardWidth);
    actionsContainer->setFixedWidth(cardWidth);
    
    // Update button widths
    updateStatusCard();
    }
}

void SubscriptionStatusPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void SubscriptionStatusPage::retranslateUI()
{
    titleLabel->setText(tr("Subscription Status"));
    subscribeButton->setText(tr("Subscribe Now"));
    renewButton->setText(tr("Renew Subscription"));
    changePlanButton->setText(tr("Change Plan"));
    
    // Update all dynamic text
    loadMemberData();
} 