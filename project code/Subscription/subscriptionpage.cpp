#include "subscriptionpage.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QSizePolicy>
#include <QScrollArea>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDebug>

SubscriptionPage::SubscriptionPage(QWidget *parent, MemberDataManager* memberManager)
    : QWidget(parent)
    , isDarkTheme(false)
    , selectedPlanId(-1)
    , currentVipCardId(-1)
    , isUpdatingStyles(false)
    , isLayoutUpdatePending(false)
    , memberManager(memberManager)
{
    setupUI();
}

void SubscriptionPage::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    contentContainer = new QWidget;
    contentContainer->setObjectName("contentContainer");
    contentContainer->setStyleSheet("QWidget#contentContainer { background: transparent; }");
    contentContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* containerLayout = new QVBoxLayout(contentContainer);
    containerLayout->setSpacing(24);
    containerLayout->setContentsMargins(32, 32, 32, 32);
    containerLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    titleLabel = new QLabel(tr("Choose Your Subscription Plan"), contentContainer);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet(UIUtils::getWelcomeLabelStyle(isDarkTheme));
    titleLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(titleLabel);

    QWidget* plansContainer = new QWidget(contentContainer);
    plansContainer->setObjectName("plansContainer");
    plansContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    plansLayout = new QHBoxLayout(plansContainer);
    plansLayout->setSpacing(20);
    plansLayout->setContentsMargins(0, 0, 0, 0);

    planButtonGroup = new QButtonGroup(this);
    planButtonGroup->setExclusive(true);

    QStringList monthlyFeatures = {
        tr("Access to all gym facilities"),
        tr("Basic workout plans"),
        tr("Email support"),
        tr("Progress tracking")
    };

    QStringList threeMonthFeatures = {
        tr("All Monthly features"),
        tr("Personalized workout plans"),
        tr("Priority email support"),
        tr("Nutrition guidance")
    };

    QStringList sixMonthFeatures = {
        tr("All 3-Month features"),
        tr("Personal trainer consultation"),
        tr("Advanced workout analytics"),
        tr("Meal planning")
    };

    QStringList yearlyFeatures = {
        tr("All 6-Month features"),
        tr("Unlimited personal training"),
        tr("VIP class booking"),
        tr("Exclusive member events")
    };

    createPlanCard(tr("Monthly"), "$29.99", tr("per month"), monthlyFeatures, 0);
    createPlanCard(tr("3 Months"), "$79.99", tr("every 3 months"), threeMonthFeatures, 1, true);
    createPlanCard(tr("6 Months"), "$149.99", tr("every 6 months"), sixMonthFeatures, 2);
    createPlanCard(tr("Yearly"), "$249.99", tr("per year"), yearlyFeatures, 3);

    containerLayout->addWidget(plansContainer);
    containerLayout->addStretch();

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    scrollArea->setWidget(contentContainer);

    mainLayout->addWidget(scrollArea);
}

void SubscriptionPage::createPlanCard(const QString& title, const QString& price, const QString& duration,
                                    const QStringList& features, int id, bool isPopular)
{
    QWidget* card = new QWidget;
    card->setObjectName(QString("planCard%1").arg(id));
    card->setMinimumWidth(300);  
    card->setMaximumWidth(340);
    card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(20); 
    cardLayout->setContentsMargins(28, 32, 28, 32); 

    // Popular badge with enhanced design
    if (isPopular) {
        QLabel* popularBadge = new QLabel(tr("Most Popular"));
        popularBadge->setObjectName("popularLabel");
        popularBadge->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED);
                color: white;
                font-size: 13px;
                font-weight: 600;
                padding: 6px 16px;
                border-radius: 20px;
                letter-spacing: 0.5px;
                margin-bottom: 8px;
            }
        )");
        popularBadge->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(popularBadge, 0, Qt::AlignHCenter);
    } else {
        cardLayout->addSpacing(32);
    }

    // Title with enhanced typography
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setObjectName("planTitle");
    titleLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 24px;
            font-weight: 700;
            letter-spacing: -0.5px;
            margin-bottom: 8px;
        }
    )").arg(isDarkTheme ? "#F9FAFB" : "#111827"));
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // Price container with enhanced layout
    QWidget* priceContainer = new QWidget;
    priceContainer->setObjectName("priceContainer");
    QVBoxLayout* priceLayout = new QVBoxLayout(priceContainer);
    priceLayout->setSpacing(8);
    priceLayout->setContentsMargins(0, 8, 0, 8);

    // Original price label with enhanced typography
    QLabel* originalPriceLabel = new QLabel(price);
    originalPriceLabel->setObjectName("originalPrice");
    originalPriceLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 36px;
            font-weight: 800;
            letter-spacing: -0.5px;
        }
    )").arg(isDarkTheme ? "#F9FAFB" : "#111827"));
    originalPriceLabel->setAlignment(Qt::AlignCenter);
    priceLayout->addWidget(originalPriceLabel);

    // VIP price label with enhanced design
    QLabel* vipPriceLabel = new QLabel;
    vipPriceLabel->setObjectName("vipPrice");
    vipPriceLabel->setStyleSheet(R"(
        QLabel {
            color: #FFD700;
            font-size: 36px;
            font-weight: 800;
            letter-spacing: -0.5px;
            text-shadow: 0 0 15px rgba(255, 215, 0, 0.4);
        }
    )");
    vipPriceLabel->setAlignment(Qt::AlignCenter);
    vipPriceLabel->hide();
    priceLayout->addWidget(vipPriceLabel);

    // Price difference label with enhanced design
    QLabel* priceDiffLabel = new QLabel;
    priceDiffLabel->setObjectName("priceDiff");
    priceDiffLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 15px;
            font-weight: 500;
            font-style: italic;
            letter-spacing: 0.2px;
            padding: 6px 12px;
            border-radius: 12px;
            background: %2;
        }
    )").arg(
        isDarkTheme ? "#A5B4FC" : "#6D28D9",
        isDarkTheme ? "rgba(139, 92, 246, 0.15)" : "rgba(139, 92, 246, 0.08)"
    ));
    priceDiffLabel->setAlignment(Qt::AlignCenter);
    priceDiffLabel->hide();
    priceLayout->addWidget(priceDiffLabel);

    cardLayout->addWidget(priceContainer);

    // Duration with enhanced typography
    QLabel* durationLabel = new QLabel(duration);
    durationLabel->setObjectName("planDuration");
    durationLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 15px;
            font-weight: 500;
            letter-spacing: 0.2px;
            margin-bottom: 8px;
        }
    )").arg(isDarkTheme ? "#9CA3AF" : "#6B7280"));
    durationLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(durationLabel);

    // Separator line
    QFrame* separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet(QString("background: %1;")
        .arg(isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"));
    cardLayout->addWidget(separator);

    // VIP Toggle container with enhanced design
    QWidget* vipContainer = new QWidget;
    QVBoxLayout* vipLayout = new QVBoxLayout(vipContainer);
    vipLayout->setSpacing(6);
    vipLayout->setContentsMargins(0, 12, 0, 12);

    QCheckBox* vipToggle = new QCheckBox(tr("VIP Upgrade"));
    vipToggle->setObjectName(QString("vipToggle%1").arg(id));
    vipToggle->setStyleSheet(QString(R"(
        QCheckBox {
            color: %1;
            font-size: 15px;
            font-weight: 600;
            spacing: 10px;
            letter-spacing: 0.2px;
        }
        QCheckBox::indicator {
            width: 22px;
            height: 22px;
            border-radius: 6px;
            border: 2px solid %2;
        }
        QCheckBox::indicator:checked {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFD700, stop:1 #FFA500);
            border: 2px solid #FFD700;
            image: url(:/Images/check.png);
        }
        QCheckBox::indicator:hover {
            border: 2px solid #FFD700;
            background-color: rgba(255, 215, 0, 0.1);
        }
        QCheckBox::indicator:pressed {
            background-color: rgba(255, 215, 0, 0.2);
        }
    )").arg(
        isDarkTheme ? "#E5E7EB" : "#1F2937",
        isDarkTheme ? "#4B5563" : "#9CA3AF"
    ));

    QLabel* vipEncouragement = new QLabel(tr("Get priority access to Padel courts and classes!"));
    vipEncouragement->setObjectName(QString("vipEncouragement%1").arg(id));
    vipEncouragement->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: 13px;
            font-weight: 500;
            padding-left: 32px;
            padding-top: 4px;
            line-height: 1.4;
            background: transparent;
        }
    )").arg(isDarkTheme ? "#9CA3AF" : "#6B7280"));
    vipEncouragement->setWordWrap(true);

    vipLayout->addWidget(vipToggle);
    vipLayout->addWidget(vipEncouragement);
    cardLayout->addWidget(vipContainer);

    // Features container with enhanced design
    QWidget* featuresContainer = new QWidget;
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresContainer);
    featuresLayout->setSpacing(12);  // Increased spacing between features
    featuresLayout->setContentsMargins(0, 16, 0, 16);

    int featureIndex = 0;
    for (const QString& feature : features) {
        QLabel* featureLabel = new QLabel(feature);
        featureLabel->setObjectName(QString("feature%1").arg(featureIndex++));
        featureLabel->setStyleSheet(QString(R"(
            QLabel {
                color: %1;
                font-size: 14px;
                font-weight: 500;
                letter-spacing: 0.2px;
                line-height: 1.4;
                padding: 4px 0;
            }
        )").arg(isDarkTheme ? "#E5E7EB" : "#4B5563"));
        featureLabel->setWordWrap(true);
        featuresLayout->addWidget(featureLabel);
    }

    cardLayout->addWidget(featuresContainer);

    // Select button with enhanced design
    QPushButton* selectButton = new QPushButton(tr("Select Plan"));
    selectButton->setObjectName("selectButton");
    selectButton->setStyleSheet(QString(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 16px 32px;
            font-size: 15px;
            font-weight: 600;
            letter-spacing: 0.3px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7C3AED, stop:1 #6D28D9);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6D28D9, stop:1 #5B21B6);
        }
    )"));
    selectButton->setCursor(Qt::PointingHandCursor);
    cardLayout->addWidget(selectButton);

    // Store references
    vipToggles[id] = vipToggle;
    vipEncouragements[id] = vipEncouragement;
    planCards[id] = card;

    // Add to button group and layout
    planButtonGroup->addButton(selectButton, id);
    plansLayout->addWidget(card);

    // Connect signals
    connect(vipToggle, &QCheckBox::toggled, [this, id](bool checked) {
        onVipToggleChanged(id, checked);
    });
    connect(selectButton, &QPushButton::clicked, [this, id]() {
        onSelectButtonClicked(id);
    });

    // Update initial style
    updateCardStyles();
}

void SubscriptionPage::updateCardStyles()
{
    if (isUpdatingStyles) return;
    isUpdatingStyles = true;

    for (auto it = planCards.begin(); it != planCards.end(); ++it) {
        QWidget* card = it.value();
        int cardId = it.key();
        bool isVip = (cardId == currentVipCardId);
        bool isSelected = (cardId == selectedPlanId);

        // Calculate prices
        double basePrice, vipPrice, vipAdditional;
        switch (cardId) {
            case 0: 
                basePrice = 29.99; 
                vipAdditional = 9.00;
                vipPrice = basePrice + vipAdditional; 
                break;
            case 1: 
                basePrice = 79.99; 
                vipAdditional = 24.00;
                vipPrice = basePrice + vipAdditional; 
                break;
            case 2: 
                basePrice = 149.99; 
                vipAdditional = 45.00;
                vipPrice = basePrice + vipAdditional; 
                break;
            case 3: 
                basePrice = 249.99; 
                vipAdditional = 75.00;
                vipPrice = basePrice + vipAdditional; 
                break;
            default: continue;
        }

        // Force update text colors for all labels
        QList<QLabel*> allLabels = card->findChildren<QLabel*>();
        for (QLabel* label : allLabels) {
            if (label->objectName() == "planTitle") {
                label->setStyleSheet(QString("color: %1;").arg(
                    isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#111827")));
            }
            else if (label->objectName() == "originalPrice" || label->objectName() == "vipPrice") {
                label->setStyleSheet(QString("color: %1;").arg(
                    isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#111827")));
            }
            else if (label->objectName() == "planDuration") {
                label->setStyleSheet(QString("color: %1;").arg(
                    isVip ? "#8B4513" : (isDarkTheme ? "#E5E7EB" : "#6B7280")));
            }
            else if (label->objectName().startsWith("feature")) {
                label->setStyleSheet(QString("color: %1;").arg(
                    isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#111827")));
            }
        }

        // Update price labels visibility
        if (QLabel* originalPrice = card->findChild<QLabel*>("originalPrice")) {
            originalPrice->setText(QString("$%1").arg(basePrice, 0, 'f', 2));
            originalPrice->setVisible(!isVip);
        }

        if (QLabel* vipPriceLabel = card->findChild<QLabel*>("vipPrice")) {
            vipPriceLabel->setText(QString("$%1").arg(vipPrice, 0, 'f', 2));
            vipPriceLabel->setVisible(isVip);
        }

        if (QLabel* priceDiffLabel = card->findChild<QLabel*>("priceDiff")) {
            priceDiffLabel->setText(QString("+$%1 VIP").arg(vipAdditional, 0, 'f', 2));
            priceDiffLabel->setVisible(isVip);
        }

        QString cardStyle = QString(
            "QWidget#%1 {"
            "   background: %2;"
            "   border-radius: 16px;"
            "   border: %3;"
            "   %4"
            "}"
            "QLabel#planTitle {"
            "   color: %5 !important;"
            "   font-size: 24px;"
            "   font-weight: 700;"
            "   letter-spacing: -0.5px;"
            "}"
            "QLabel#originalPrice, QLabel#vipPrice {"
            "   color: %5 !important;"
            "   font-size: 36px;"
            "   font-weight: 800;"
            "   letter-spacing: -0.5px;"
            "}"
            "QLabel#priceDiff {"
            "   color: %6 !important;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   letter-spacing: 0.2px;"
            "   padding: 4px 12px;"
            "   border-radius: 12px;"
            "   background: %7;"
            "   margin-top: 4px;"
            "   margin-bottom: 8px;"
            "}"
            "QLabel#planDuration {"
            "   color: %8 !important;"
            "   font-size: 15px;"
            "   font-weight: 500;"
            "   letter-spacing: 0.2px;"
            "   margin-top: 8px;"
            "}"
            "QLabel[objectName^='feature'] {"
            "   color: %9 !important;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   letter-spacing: 0.2px;"
            "   line-height: 1.4;"
            "   padding: 4px 0;"
            "}"
        ).arg(
            card->objectName(),
            // Background color
            isVip ? (isDarkTheme ? 
                "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FFD700, stop:0.5 #DAA520, stop:1 #FFD700)" :
                "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #FFE55C, stop:0.5 #FFD700, stop:1 #FFE55C)") :
            (isDarkTheme ? "rgba(31, 41, 55, 0.98)" : "rgba(255, 255, 255, 0.98)"),
            // Border color
            isSelected ? 
                "2px solid #8B5CF6" : 
                (isVip ? "2px solid #B8860B" : 
                    (isDarkTheme ? "2px solid rgba(255, 255, 255, 0.1)" : "2px solid rgba(0, 0, 0, 0.05)")),
            // Box shadow
            isVip ? 
                "box-shadow: 0 8px 24px rgba(218, 165, 32, 0.3);" : 
                (isDarkTheme ? "box-shadow: 0 8px 24px rgba(0, 0, 0, 0.3);" : "box-shadow: 0 8px 24px rgba(0, 0, 0, 0.1);"),
            // Title and price colors
            isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#111827"),
            // VIP additional price text color
            isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#8B5CF6"),
            // VIP additional price background
            isVip ? "rgba(139, 69, 19, 0.1)" : (isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(139, 92, 246, 0.1)"),
            // Duration color
            isVip ? "#8B4513" : (isDarkTheme ? "#E5E7EB" : "#6B7280"),
            // Feature text color
            isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#111827")
        );

        card->setStyleSheet(cardStyle);

        // Update VIP toggle style
        if (QCheckBox* vipToggle = vipToggles.value(cardId)) {
            vipToggle->setStyleSheet(QString(
                "QCheckBox {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 600;"
                "   spacing: 10px;"
                "   letter-spacing: 0.2px;"
                "}"
                "QCheckBox::indicator {"
                "   width: 22px;"
                "   height: 22px;"
                "   border-radius: 6px;"
                "   border: 2px solid %2;"
                "   background-color: %3;"
                "}"
                "QCheckBox::indicator:checked {"
                "   background: %4;"
                "   border: 2px solid #B8860B;"
                "   image: url(:/Images/check.png);"
                "}"
                "QCheckBox::indicator:hover {"
                "   border: 2px solid #B8860B;"
                "   background-color: rgba(184, 134, 11, 0.1);"
                "}"
                "QCheckBox::indicator:pressed {"
                "   background-color: rgba(184, 134, 11, 0.2);"
                "}"
            ).arg(
                isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#4B5563"),
                isVip ? "#B8860B" : (isDarkTheme ? "#E5E7EB" : "#9CA3AF"),
                isVip ? "rgba(255, 255, 255, 0.9)" : "transparent",
                isVip ? "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #DAA520, stop:1 #B8860B)" :
                       "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED)"
            ));
        }

        // Update VIP encouragement text
        if (QLabel* vipEncouragement = vipEncouragements.value(cardId)) {
            vipEncouragement->setStyleSheet(QString(
                "QLabel {"
                "   color: %1 !important;"
                "   font-size: 13px;"
                "   font-weight: 500;"
                "   padding-left: 32px;"
                "   padding-top: 4px;"
                "   line-height: 1.4;"
                "   background: transparent;"
                "}"
            ).arg(isVip ? "#8B4513" : (isDarkTheme ? "#FFFFFF" : "#6B7280")));
        }
    }

    isUpdatingStyles = false;
}

void SubscriptionPage::onVipToggleChanged(int cardId, bool checked)
{
    if (checked) {
        // If another card is VIP, uncheck it
        if (currentVipCardId != -1 && currentVipCardId != cardId) {
            vipToggles[currentVipCardId]->setChecked(false);
        }
        currentVipCardId = cardId;
    } else if (currentVipCardId == cardId) {
        currentVipCardId = -1;
    }
    updateCardStyles();
}

void SubscriptionPage::onPlanSelected(int id)
{
    if (selectedPlanId != -1) {
        animateCardSelection(planCards[selectedPlanId], false);
    }
    selectedPlanId = id;
    animateCardSelection(planCards[id], true);
    updateCardStyles();
}

void SubscriptionPage::onSelectButtonClicked(int id)
{
    selectedPlanId = id;  // Update the selected plan
    bool isVip = (id == currentVipCardId);  // Check if VIP is enabled for this plan
    
    if (!isVip) {
        // If VIP is not selected, show confirmation dialog
        showVipConfirmationDialog(id);
    } else {
        // If VIP is already selected, proceed directly
        emit paymentRequested(id, true);
    }
    
    updateCardStyles();  // Update the visual state
}

void SubscriptionPage::showVipConfirmationDialog(int planId)
{
    // Create a new dialog each time to ensure it's properly initialized
    if (vipConfirmationDialog) {
        delete vipConfirmationDialog;
    }
    
    vipConfirmationDialog = new QMessageBox(this);
    vipConfirmationDialog->setWindowTitle(tr("VIP Option Available"));
    vipConfirmationDialog->setIcon(QMessageBox::Information);
    
    // Create custom buttons with clear text
    QPushButton* upgradeButton = new QPushButton(tr("Add VIP Upgrade"));
    QPushButton* skipButton = new QPushButton(tr("Continue Without VIP"));
    
    // Store the buttons for later identification
    upgradeButton->setObjectName("upgradeButton");
    skipButton->setObjectName("skipButton");
    
    // Add buttons to dialog
    vipConfirmationDialog->addButton(upgradeButton, QMessageBox::AcceptRole);
    vipConfirmationDialog->addButton(skipButton, QMessageBox::RejectRole);
    
    // Style the buttons based on theme
    QString upgradeButtonStyle = QString(
        "QPushButton {"
        "  background: %1;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: %2;"
        "}"
        "QPushButton:pressed {"
        "  background: %3;"
        "}"
    ).arg(
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFD700, stop:1 #DAA520)",
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FFE55C, stop:1 #FFD700)",
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #DAA520, stop:1 #B8860B)"
    );
    
    QString skipButtonStyle = QString(
        "QPushButton {"
        "  background: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px 16px;"
        "  font-weight: 600;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background: %3;"
        "}"
        "QPushButton:pressed {"
        "  background: %4;"
        "}"
    ).arg(
        isDarkTheme ? "#374151" : "#F3F4F6",
        isDarkTheme ? "#FFFFFF" : "#111827",
        isDarkTheme ? "#4B5563" : "#E5E7EB",
        isDarkTheme ? "#1F2937" : "#D1D5DB"
    );
    
    upgradeButton->setStyleSheet(upgradeButtonStyle);
    skipButton->setStyleSheet(skipButtonStyle);
    
    // Calculate VIP price based on plan
    double vipPrice = 0.0;
    switch (planId) {
        case 0: vipPrice = 9.00; break;
        case 1: vipPrice = 24.00; break;
        case 2: vipPrice = 45.00; break;
        case 3: vipPrice = 75.00; break;
    }
    
    // Create a more beautiful message with HTML formatting (no background colors)
    QString messageText = QString(
        "<div style='font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto; padding: 16px 0;'>"
        "<h2 style='margin: 0 0 20px 0; color: %1; font-size: 20px; letter-spacing: -0.3px; font-weight: 700; text-align: center;'>"
        "✨ %2 ✨"
        "</h2>"
        "<div style='margin: 0 0 20px 0; text-align: center;'>"
        "<span style='color: %3; font-size: 36px; font-weight: 800; letter-spacing: -0.5px;'>$%4</span>"
        "<span style='color: %5; font-size: 16px; font-weight: 500; margin-left: 5px;'>%6</span>"
        "</div>"
        "<div style='margin: 24px 0 16px 0; border-left: 3px solid %7; padding-left: 16px;'>"
        "<p style='color: %8; font-size: 15px; line-height: 1.6; font-weight: 500; margin: 0 0 8px 0;'>"
        "%9"
        "</p>"
        "</div>"
        "<ul style='color: %10; line-height: 1.6; margin: 16px 0; padding-left: 24px; font-size: 14.5px;'>"
        "<li style='margin-bottom: 12px;'>%11</li>"
        "<li style='margin-bottom: 12px;'>%12</li>"
        "<li style='margin-bottom: 12px;'>%13</li>"
        "</ul>"
        "<p style='margin: 20px 0 0 0; color: %14; font-size: 13px; text-align: center; font-style: italic;'>"
        "🌟 %15"
        "</p>"
        "</div>"
    ).arg(
        isDarkTheme ? "#FFFFFF" : "#111827",                          // Title color
        tr("Upgrade to VIP for Premium Benefits"),                    // Title text
        isDarkTheme ? "#FFD700" : "#B8860B",                          // Price color
        QString::number(vipPrice, 'f', 2),                            // Price value
        isDarkTheme ? "#9CA3AF" : "#6B7280",                          // Price label color 
        tr("additional"),                                             // Price label text
        isDarkTheme ? "#FFD700" : "#B8860B",                          // Left border color
        isDarkTheme ? "#E5E7EB" : "#374151",                          // Subtitle text color
        tr("Enhance your experience with these exclusive VIP benefits:"), // Subtitle text
        isDarkTheme ? "#E5E7EB" : "#4B5563",                          // List text color
        tr("Priority booking for all Padel courts"),                  // Benefit 1
        tr("Exclusive access to VIP-only classes and events"),        // Benefit 2
        tr("Premium customer support with dedicated VIP line"),       // Benefit 3
        isDarkTheme ? "#A78BFA" : "#8B5CF6",                          // Note color
        tr("Join our elite members today!")                           // Note text
    );
    
    vipConfirmationDialog->setText(messageText);
    
    // Apply theme-aware styling to the dialog
    vipConfirmationDialog->setStyleSheet(QString(
        "QMessageBox {"
        "  background-color: %1;"
        "  border-radius: 12px;"
        "  min-width: 420px;"
        "}"
        "QMessageBox QLabel {"
        "  background-color: transparent;"
        "  color: %2;"
        "}"
    ).arg(
        isDarkTheme ? "#1F2937" : "#FFFFFF",
        isDarkTheme ? "#E5E7EB" : "#374151"
    ));
    
    // Connect button signals directly
    connect(upgradeButton, &QPushButton::clicked, this, [this, planId]() {
        // Turn on VIP toggle
        if (QCheckBox* vipToggle = vipToggles.value(planId)) {
            vipToggle->setChecked(true);
            currentVipCardId = planId;
            updateCardStyles();
        }
        // Navigate to payment page with VIP
        emit paymentRequested(planId, true);
        vipConfirmationDialog->close();
    });
    
    connect(skipButton, &QPushButton::clicked, this, [this, planId]() {
        // Navigate to payment page without VIP
        emit paymentRequested(planId, false);
        vipConfirmationDialog->close();
    });
    
    // Show the dialog
    vipConfirmationDialog->exec();
}

void SubscriptionPage::animateCardSelection(QWidget* card, bool selected)
{
    QPropertyAnimation* scaleAnimation = new QPropertyAnimation(card, "geometry");
    scaleAnimation->setDuration(200);
    scaleAnimation->setEasingCurve(QEasingCurve::OutBack);

    QRect startGeometry = card->geometry();
    QRect endGeometry = startGeometry;

    if (selected) {
        endGeometry.adjust(-10, -10, 10, 10);
    }

    scaleAnimation->setStartValue(startGeometry);
    scaleAnimation->setEndValue(endGeometry);
    scaleAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SubscriptionPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Update main title color
    titleLabel->setStyleSheet(QString("color: %1; font-size: 28px; font-weight: 700;")
        .arg(isDark ? "#F9FAFB" : "#111827"));

    // Update "Most Popular" badge if it exists
    if (QLabel* popularLabel = contentContainer->findChild<QLabel*>("popularLabel")) {
        popularLabel->setStyleSheet(R"(
            QLabel {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED);
                color: white;
                font-size: 12px;
                font-weight: 600;
                padding: 4px 12px;
                border-radius: 12px;
            }
        )");
    }
    
    // Update VIP confirmation dialog if it exists
    if (vipConfirmationDialog) {
        // Update the buttons
        QList<QPushButton*> buttons = vipConfirmationDialog->findChildren<QPushButton*>();
        if (buttons.size() >= 2) {
            buttons[1]->setStyleSheet(QString(
                "QPushButton {"
                "  background: %1;"
                "  color: %2;"
                "  border: none;"
                "  border-radius: 8px;"
                "  padding: 10px 16px;"
                "  font-weight: 600;"
                "  font-size: 14px;"
                "}"
                "QPushButton:hover {"
                "  background: %3;"
                "}"
                "QPushButton:pressed {"
                "  background: %4;"
                "}"
            ).arg(
                isDarkTheme ? "#374151" : "#F3F4F6",
                isDarkTheme ? "#FFFFFF" : "#111827",
                isDarkTheme ? "#4B5563" : "#E5E7EB",
                isDarkTheme ? "#1F2937" : "#D1D5DB"
            ));
        }
        
        // Update dialog background and text colors
        vipConfirmationDialog->setStyleSheet(QString(
            "QMessageBox {"
            "  background-color: %1;"
            "  border-radius: 12px;"
            "}"
            "QMessageBox QLabel {"
            "  background-color: transparent;"
            "  color: %2;"
            "}"
            "QMessageBox QLabel#qt_msgbox_informativelabel {"
            "  min-width: 400px;"
            "}"
        ).arg(
            isDarkTheme ? "#1F2937" : "#FFFFFF",
            isDarkTheme ? "#E5E7EB" : "#374151"
        ));
    }

    // Update all card styles
    updateCardStyles();
}

void SubscriptionPage::updateLayout()
{
    if (isUpdatingStyles || isLayoutUpdatePending) return;
    isLayoutUpdatePending = true;

    const QSize size = this->size();
    const qreal baseWidth = 1400.0;
    const qreal baseHeight = 800.0;
    const qreal minScale = 0.5;
    const qreal maxScale = 0.9;
    
    const qreal widthRatio = size.width() / baseWidth;
    const qreal heightRatio = size.height() / baseHeight;
    const qreal scale = qMin(qMin(widthRatio, heightRatio), maxScale);
    const qreal finalScale = qMax(scale, minScale);

    // Calculate card dimensions
    int cardSpacing = plansLayout->spacing();
    int totalSpacing = cardSpacing * 3;
    int availableWidth = size.width() - (2 * 20) - totalSpacing;
    int cardWidth;

    if (size.width() < 600) {
        plansLayout->setDirection(QBoxLayout::TopToBottom);
        cardWidth = qMin(320, size.width() - 40);
    } else if (size.width() < 1000) {
        plansLayout->setDirection(QBoxLayout::LeftToRight);
        cardWidth = (availableWidth / 2) - cardSpacing;
    } else {
        plansLayout->setDirection(QBoxLayout::LeftToRight);
        cardWidth = (availableWidth / 4) - cardSpacing;
    }

    // Update each card's layout
    for (auto it = planCards.begin(); it != planCards.end(); ++it) {
        updateCardLayout(it.value(), cardWidth);
    }

    // Update title
    int titleFontSize = qMax(20, int(28 * finalScale));
    titleLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            font-size: %2px;
            font-weight: 700;
            letter-spacing: -0.5px;
            margin-bottom: 16px;
        }
    )").arg(
        isDarkTheme ? "#FFFFFF" : "#111827",
        QString::number(titleFontSize)
    ));

    isLayoutUpdatePending = false;
}

void SubscriptionPage::updateCardLayout(QWidget* card, int cardWidth)
{
    if (!card) return;

    card->setFixedWidth(cardWidth);
    
    if (QVBoxLayout* cardLayout = qobject_cast<QVBoxLayout*>(card->layout())) {
        int margin = cardWidth < 300 ? 16 : 24;
        cardLayout->setContentsMargins(margin, margin, margin, margin);
        cardLayout->setSpacing(margin / 2);
    }
}

void SubscriptionPage::retranslateUI()
{
    titleLabel->setText(tr("Choose Your Subscription Plan"));
    
    for (auto it = vipEncouragements.begin(); it != vipEncouragements.end(); ++it) {
        it.value()->setText(tr("Get priority access to Padel courts and classes!"));
    }

    for (auto it = vipToggles.begin(); it != vipToggles.end(); ++it) {
        it.value()->setText(tr("VIP Upgrade"));
    }
    
    if (vipConfirmationDialog) {
        vipConfirmationDialog->setWindowTitle(tr("VIP Option Available"));
        
        QList<QPushButton*> buttons = vipConfirmationDialog->findChildren<QPushButton*>();
        if (buttons.size() >= 2) {
            buttons[0]->setText(tr("Add VIP Upgrade"));
            buttons[1]->setText(tr("Continue Without VIP"));
        }
        
        if (vipConfirmationDialog->isVisible() && selectedPlanId >= 0) {
            double vipPrice = 0.0;
            switch (selectedPlanId) {
                case 0: vipPrice = 9.00; break;
                case 1: vipPrice = 24.00; break;
                case 2: vipPrice = 45.00; break;
                case 3: vipPrice = 75.00; break;
            }
            
            QString messageText = QString(
                "<div style='font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto; padding: 16px 0;'>"
                "<h2 style='margin: 0 0 20px 0; color: %1; font-size: 20px; letter-spacing: -0.3px; font-weight: 700; text-align: center;'>"
                "✨ %2 ✨"
                "</h2>"
                "<div style='margin: 0 0 20px 0; text-align: center;'>"
                "<span style='color: %3; font-size: 36px; font-weight: 800; letter-spacing: -0.5px;'>$%4</span>"
                "<span style='color: %5; font-size: 16px; font-weight: 500; margin-left: 5px;'>%6</span>"
                "</div>"
                "<div style='margin: 24px 0 16px 0; border-left: 3px solid %7; padding-left: 16px;'>"
                "<p style='color: %8; font-size: 15px; line-height: 1.6; font-weight: 500; margin: 0 0 8px 0;'>"
                "%9"
                "</p>"
                "</div>"
                "<ul style='color: %10; line-height: 1.6; margin: 16px 0; padding-left: 24px; font-size: 14.5px;'>"
                "<li style='margin-bottom: 12px;'>%11</li>"
                "<li style='margin-bottom: 12px;'>%12</li>"
                "<li style='margin-bottom: 12px;'>%13</li>"
                "</ul>"
                "<p style='margin: 20px 0 0 0; color: %14; font-size: 13px; text-align: center; font-style: italic;'>"
                "🌟 %15"
                "</p>"
                "</div>"
            ).arg(
                isDarkTheme ? "#FFFFFF" : "#111827",
                tr("Upgrade to VIP for Premium Benefits"),
                isDarkTheme ? "#FFD700" : "#B8860B",
                QString::number(vipPrice, 'f', 2),
                isDarkTheme ? "#9CA3AF" : "#6B7280",
                tr("additional"),
                isDarkTheme ? "#FFD700" : "#B8860B",
                isDarkTheme ? "#E5E7EB" : "#374151",
                tr("Enhance your experience with these exclusive VIP benefits:"),
                isDarkTheme ? "#E5E7EB" : "#4B5563",
                tr("Priority booking for all Padel courts"),
                tr("Exclusive access to VIP-only classes and events"),
                tr("Premium customer support with dedicated VIP line"),
                isDarkTheme ? "#A78BFA" : "#8B5CF6",
                tr("Join our elite members today!")
            );
            
            vipConfirmationDialog->setText(messageText);
        }
    }

    struct PlanDetails {
        QString title;
        QString price;
        QString duration;
        QStringList features;
    };

    QMap<int, PlanDetails> plans;
    
    plans[0] = {
        tr("Monthly"),
        "$29.99",
        tr("per month"),
        {
            tr("Access to all gym facilities"),
            tr("Basic workout plans"),
            tr("Email support"),
            tr("Progress tracking")
        }
    };

    plans[1] = {
        tr("3 Months"),
        "$79.99",
        tr("every 3 months"),
        {
            tr("All Monthly features"),
            tr("Personalized workout plans"),
            tr("Priority email support"),
            tr("Nutrition guidance")
        }
    };

    plans[2] = {
        tr("6 Months"),
        "$149.99",
        tr("every 6 months"),
        {
            tr("All 3-Month features"),
            tr("Personal trainer consultation"),
            tr("Advanced workout analytics"),
            tr("Meal planning")
        }
    };

    plans[3] = {
        tr("Yearly"),
        "$249.99",
        tr("per year"),
        {
            tr("All 6-Month features"),
            tr("Unlimited personal training"),
            tr("VIP class booking"),
            tr("Exclusive member events")
        }
    };

    for (auto it = planCards.begin(); it != planCards.end(); ++it) {
        QWidget* card = it.value();
        int planId = it.key();
        
        if (!plans.contains(planId)) continue;
        
        if (QLabel* titleLabel = card->findChild<QLabel*>("planTitle")) {
            titleLabel->setText(plans[planId].title);
        }
        
        if (QLabel* priceLabel = card->findChild<QLabel*>("originalPrice")) {
            priceLabel->setText(plans[planId].price);
        }
        
        if (QLabel* durationLabel = card->findChild<QLabel*>("planDuration")) {
            durationLabel->setText(plans[planId].duration);
        }
        
        QWidget* featuresContainer = nullptr;
        QList<QWidget*> childWidgets = card->findChildren<QWidget*>();
        for (QWidget* widget : childWidgets) {
            QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(widget->layout());
            if (layout && layout->count() >= plans[planId].features.size()) {
                bool isFeatureContainer = true;
                for (int i = 0; i < layout->count() && i < plans[planId].features.size(); ++i) {
                    QLabel* label = qobject_cast<QLabel*>(layout->itemAt(i)->widget());
                    if (!label || !label->objectName().startsWith("feature")) {
                        isFeatureContainer = false;
                        break;
                    }
                }
                if (isFeatureContainer) {
                    featuresContainer = widget;
                    break;
                }
            }
        }
        
        if (featuresContainer) {
            QList<QLabel*> featureLabels = featuresContainer->findChildren<QLabel*>();
            std::sort(featureLabels.begin(), featureLabels.end(), 
                     [](QLabel* a, QLabel* b) {
                         QString nameA = a->objectName();
                         QString nameB = b->objectName();
                         if (nameA.startsWith("feature") && nameB.startsWith("feature")) {
                             int numA = nameA.mid(7).toInt();
                             int numB = nameB.mid(7).toInt();
                             return numA < numB;
                         }
                         return nameA < nameB;
                     });
                     
            for (int i = 0; i < featureLabels.size() && i < plans[planId].features.size(); ++i) {
                if (featureLabels[i]->objectName().startsWith("feature")) {
                    featureLabels[i]->setText(plans[planId].features[i]);
                }
            }
        } else {
            QList<QLabel*> allLabels = card->findChildren<QLabel*>();
            for (int i = 0; i < plans[planId].features.size(); ++i) {
                QString featureName = QString("feature%1").arg(i);
                for (QLabel* label : allLabels) {
                    if (label->objectName() == featureName) {
                        label->setText(plans[planId].features[i]);
                        break;
                    }
                }
            }
        }
        
        if (QPushButton* selectButton = card->findChild<QPushButton*>("selectButton")) {
            selectButton->setText(tr("Select Plan"));
        }
    }

    QList<QLabel*> labels = contentContainer->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->objectName() == "popularLabel") {
            label->setText(tr("Most Popular"));
            break;
        }
    }

    updateCardStyles();
    updatePlanCardTexts();
}

void SubscriptionPage::updatePlanCardTexts()
{
    // Define plan features with translations
    QMap<int, QStringList> planFeatures;
    
    // Monthly Plan features
    planFeatures[0] = {
        tr("Access to all gym facilities"),
        tr("Basic workout plans"),
        tr("Email support"),
        tr("Progress tracking")
    };

    // 3-Month Plan features
    planFeatures[1] = {
        tr("All Monthly features"),
        tr("Personalized workout plans"),
        tr("Priority email support"),
        tr("Nutrition guidance")
    };

    // 6-Month Plan features
    planFeatures[2] = {
        tr("All 3-Month features"),
        tr("Personal trainer consultation"),
        tr("Advanced workout analytics"),
        tr("Meal planning")
    };

    // Yearly Plan features
    planFeatures[3] = {
        tr("All 6-Month features"),
        tr("Unlimited personal training"),
        tr("VIP class booking"),
        tr("Exclusive member events")
    };

    // Update each plan card's features
    for (auto it = planCards.begin(); it != planCards.end(); ++it) {
        QWidget* card = it.value();
        int planId = it.key();
        
        if (!planFeatures.contains(planId)) continue;
        
        // Find all feature labels in the card
        QList<QLabel*> allLabels = card->findChildren<QLabel*>();
        for (int i = 0; i < planFeatures[planId].size(); ++i) {
            QString featureName = QString("feature%1").arg(i);
            for (QLabel* label : allLabels) {
                if (label->objectName() == featureName) {
                    label->setText(planFeatures[planId][i]);
                    break;
                }
            }
        }
    }
}

void SubscriptionPage::resizeEvent(QResizeEvent* event)
{
    if (isUpdatingStyles || isLayoutUpdatePending) {
        QWidget::resizeEvent(event);
        return;
    }
    
    QWidget::resizeEvent(event);
    updateLayout();
}

void SubscriptionPage::handlePaymentCompleted(int planId, bool isVip)
{
    if (!memberManager || currentMemberId == 0) {
        qDebug() << "Cannot handle payment completion: Invalid member or manager";
        return;
    }

    // Create subscription
    createSubscription(planId, isVip);
}

void SubscriptionPage::createSubscription(int planId, bool isVip)
{
    // Determine subscription type based on plan ID
    SubscriptionType type;
    switch (planId) {
        case 0: type = SubscriptionType::MONTHLY; break;
        case 1: type = SubscriptionType::THREE_MONTHS; break;
        case 2: type = SubscriptionType::SIX_MONTHS; break;
        case 3: type = SubscriptionType::YEARLY; break;
        default: return;
    }

    // Create new subscription
    Subscription subscription(type, QDate::currentDate());
    
    QString errorMessage;
    if (!memberManager->addSubscription(currentMemberId, subscription, errorMessage)) {
        QMessageBox::warning(this, tr("Subscription Error"),
            tr("Failed to create subscription: %1").arg(errorMessage));
        return;
    }

    // Update VIP status if needed
    if (isVip) {
        memberManager->renewSubscription(currentMemberId, type, true, errorMessage);
    }
}

void SubscriptionPage::setCurrentMemberId(int memberId)
{
    currentMemberId = memberId;
} 