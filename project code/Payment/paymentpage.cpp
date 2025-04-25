#include "paymentpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QSizePolicy>
#include <QScrollArea>
#include <QEvent>
#include <QResizeEvent>
#include <QDate>
#include <QMessageBox>
#include <QRegularExpression>
#include <QThread>
#include <QProgressDialog>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QCheckBox>
#include <QPixmap>

#include "../../UI/UIUtils.h"
#include "../../Theme/ThemeManager.h"
#include "../DataManager/memberdatamanager.h"
#include "../DataManager/userdatamanager.h"
#include "../Model/Gym/subscription.h"
#include "../Language/LanguageManager.h"

PaymentPage::PaymentPage(QWidget *parent, MemberDataManager* memberManager, UserDataManager* userDataManager)
    : QWidget(parent)
    , isDarkTheme(ThemeManager::getInstance().isDarkTheme())
    , selectedPlanId(0)
    , isVipEnabled(false)
    , isUpdatingStyles(false)
    , isLayoutUpdatePending(false)
    , mainLayout(nullptr)
    , contentContainer(nullptr)
    , titleLabel(nullptr)
    , paymentCard(nullptr)
    , cardNumberInput(nullptr)
    , expiryDateInput(nullptr)
    , cvvInput(nullptr)
    , nameInput(nullptr)
    , payButton(nullptr)
    , backButton(nullptr)
    , planPriceLabel(nullptr)
    , vipPriceLabel(nullptr)
    , totalPriceLabel(nullptr)
    , monthCombo(nullptr)
    , yearCombo(nullptr)
    , cvcDialog(nullptr)
    , memberManager(memberManager)
    , userDataManager(userDataManager)
    , currentMemberId(0)
    , currentUserId(0)
    , hasSavedCard(false)
{
    setupUI();
    setupCVCDialog();
    
    // Connect to ThemeManager for theme updates
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, &PaymentPage::updateTheme);
            
    // Connect to LanguageManager for language changes
    connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
            this, [this](const QString&) {
                retranslateUI();
                updateLayout();
            });

    // Force initial theme application
    updateTheme(isDarkTheme);
    retranslateUI();
}

void PaymentPage::setupUI()
{
    // Restore some top margin and add spacing between title and content
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 24);
    mainLayout->setSpacing(16);

    // Title with proper margins
    titleLabel = new QLabel(this);
    titleLabel->setAlignment(Qt::AlignLeft);
    titleLabel->setObjectName("pageTitle");
    mainLayout->addWidget(titleLabel);

    // Create a scroll area for responsive design
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("paymentScrollArea");
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

    // Content Container without top padding since we have spacing in main layout
    contentContainer = new QWidget();
    contentContainer->setObjectName("contentContainer");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(20);

    // Payment Card Container
    paymentCard = new QWidget(contentContainer);
    paymentCard->setObjectName("paymentCard");
    QVBoxLayout* paymentCardLayout = new QVBoxLayout(paymentCard);
    paymentCardLayout->setSpacing(24);
    paymentCardLayout->setContentsMargins(24, 24, 24, 24);

    // Receipt Summary Section
    QWidget* receiptSection = new QWidget(paymentCard);
    receiptSection->setObjectName("receiptSection");
    QVBoxLayout* receiptLayout = new QVBoxLayout(receiptSection);
    receiptLayout->setSpacing(16);
    receiptLayout->setContentsMargins(24, 24, 24, 24);

    // Basic Plan Details
    QHBoxLayout* planLayout = new QHBoxLayout();
    QLabel* planLabel = new QLabel(tr("Standard Plan:"), receiptSection);
    planLabel->setObjectName("receiptLabel");
    planPriceLabel = new QLabel("$0.00", receiptSection);
    planPriceLabel->setObjectName("receiptPrice");
    planLayout->addWidget(planLabel);
    planLayout->addStretch();
    planLayout->addWidget(planPriceLabel);
    receiptLayout->addLayout(planLayout);

    // VIP Option
    QHBoxLayout* vipLayout = new QHBoxLayout();
    QLabel* vipLabel = new QLabel(tr("VIP Option:"), receiptSection);
    vipLabel->setObjectName("receiptLabel");
    vipPriceLabel = new QLabel("$0.00", receiptSection);
    vipPriceLabel->setObjectName("vipPrice");
    vipLayout->addWidget(vipLabel);
    vipLayout->addStretch();
    vipLayout->addWidget(vipPriceLabel);
    receiptLayout->addLayout(vipLayout);

    // Divider
    QFrame* divider = new QFrame(receiptSection);
    divider->setFrameShape(QFrame::HLine);
    divider->setObjectName("divider");
    receiptLayout->addWidget(divider);

    // Total
    QHBoxLayout* totalLayout = new QHBoxLayout();
    QLabel* totalLabel = new QLabel(tr("TOTAL:"), receiptSection);
    totalLabel->setObjectName("totalLabel");
    totalPriceLabel = new QLabel("$0.00", receiptSection);
    totalPriceLabel->setObjectName("totalPrice");
    totalLayout->addWidget(totalLabel);
    totalLayout->addStretch();
    totalLayout->addWidget(totalPriceLabel);
    receiptLayout->addLayout(totalLayout);

    paymentCardLayout->addWidget(receiptSection);

    // Payment Information Section
    QWidget* paymentSection = new QWidget(paymentCard);
    paymentSection->setObjectName("paymentSection");
    QVBoxLayout* paymentLayout = new QVBoxLayout(paymentSection);
    paymentLayout->setSpacing(16);
    paymentLayout->setContentsMargins(0, 0, 0, 0);

    // Payment Section Main Container
    QWidget* cardRowContainer = new QWidget(paymentSection);
    QHBoxLayout* cardRowLayout = new QHBoxLayout(cardRowContainer);
    cardRowLayout->setContentsMargins(0, 0, 0, 0);
    cardRowLayout->setSpacing(20);

    // Card Number Section
    QWidget* cardNumberSection = new QWidget(cardRowContainer);
    cardNumberSection->setObjectName("cardNumberSection");
    QVBoxLayout* cardNumberLayout = new QVBoxLayout(cardNumberSection);
    cardNumberLayout->setContentsMargins(0, 0, 0, 0);
    cardNumberLayout->setSpacing(6);
    
    QLabel* cardNumberLabel = new QLabel(tr("Card Number"), cardNumberSection);
    cardNumberLabel->setObjectName("fieldLabel");
    
    QWidget* cardNumberContainer = new QWidget(cardNumberSection);
    cardNumberContainer->setObjectName("cardNumberContainer");
    QHBoxLayout* cardNumberInputLayout = new QHBoxLayout(cardNumberContainer);
    cardNumberInputLayout->setContentsMargins(0, 0, 0, 0);
    cardNumberInputLayout->setSpacing(12);
    
    cardNumberInput = new QLineEdit(cardNumberContainer);
    cardNumberInput->setPlaceholderText("1234 5678 9012 3456");
    cardNumberInput->setObjectName("cardNumberInput");
    cardNumberInput->setMinimumHeight(40);
    cardNumberInput->setMaxLength(19);
    
    // Auto format card number
    connect(cardNumberInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString filtered = text;
        filtered.remove(' ');
        filtered.remove(QRegularExpression("[^0-9]"));
        
        QString formatted;
        for (int i = 0; i < filtered.length(); ++i) {
            if (i > 0 && i % 4 == 0) {
                formatted += ' ';
            }
            formatted += filtered[i];
        }
        
        if (formatted != text) {
            cardNumberInput->setText(formatted);
            cardNumberInput->setCursorPosition(formatted.length());
        }
    });

    QLabel* cardLogo = new QLabel(cardNumberContainer);
    cardLogo->setPixmap(UIUtils::getIcon("mastercard", 24));
    cardLogo->setFixedSize(24, 24);
    
    cardNumberInputLayout->addWidget(cardNumberInput);
    cardNumberInputLayout->addWidget(cardLogo);
    
    cardNumberLayout->addWidget(cardNumberLabel);
    cardNumberLayout->addWidget(cardNumberContainer);

    // Name on Card Section (in same row)
    QWidget* nameSection = new QWidget(cardRowContainer);
    nameSection->setObjectName("nameSection");
    QVBoxLayout* nameLayout = new QVBoxLayout(nameSection);
    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(6);
    
    QLabel* nameLabel = new QLabel(tr("Name on Card"), nameSection);
    nameLabel->setObjectName("fieldLabel");
    
    nameInput = new QLineEdit(nameSection);
    nameInput->setPlaceholderText(tr("James Zapata"));
    nameInput->setObjectName("nameInput");
    nameInput->setMinimumHeight(40);
    
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(nameInput);
    
    // Add the card number and name sections to the row
    cardRowLayout->addWidget(cardNumberSection, 3);  // Wider for card number
    cardRowLayout->addWidget(nameSection, 2);        // Narrower for name
    
    // Add the row to the main payment layout
    paymentLayout->addWidget(cardRowContainer);
    paymentLayout->addSpacing(12);

    // Card Details - Second Row: Expiry Date + CVC
    QWidget* cardDetailsContainer = new QWidget(paymentSection);
    QHBoxLayout* cardDetailsLayout = new QHBoxLayout(cardDetailsContainer);
    cardDetailsLayout->setContentsMargins(0, 0, 0, 0);
    cardDetailsLayout->setSpacing(20);

    // Expiry Date
    QWidget* expiryContainer = new QWidget(cardDetailsContainer);
    expiryContainer->setObjectName("expiryContainer");
    QVBoxLayout* expiryLayout = new QVBoxLayout(expiryContainer);
    expiryLayout->setContentsMargins(0, 0, 0, 0);
    expiryLayout->setSpacing(6);

    QLabel* expiryLabel = new QLabel(tr("Expiry Date"), expiryContainer);
    expiryLabel->setObjectName("fieldLabel");
    
    expiryDateInput = new QLineEdit(expiryContainer);
    expiryDateInput->setPlaceholderText("MM / YY");
    expiryDateInput->setObjectName("expiryDateInput");
    expiryDateInput->setMaxLength(7);
    expiryDateInput->setMinimumHeight(40);
    expiryDateInput->setFixedWidth(100);

    // Auto format expiry date with validation
    connect(expiryDateInput, &QLineEdit::textChanged, this, &PaymentPage::validateExpiryDate);

    expiryLayout->addWidget(expiryLabel);
    expiryLayout->addWidget(expiryDateInput);

    // CVC - Fixed the label to correctly say "CVC"
    QWidget* cvcContainer = new QWidget(cardDetailsContainer);
    cvcContainer->setObjectName("cvcContainer");
    QVBoxLayout* cvcLayout = new QVBoxLayout(cvcContainer);
    cvcLayout->setContentsMargins(0, 0, 0, 0);
    cvcLayout->setSpacing(6);

    QLabel* cvcLabel = new QLabel(tr("CVC"), cvcContainer);
    cvcLabel->setObjectName("fieldLabel");
    
    QHBoxLayout* cvcInputLayout = new QHBoxLayout();
    cvcInputLayout->setContentsMargins(0, 0, 0, 0);
    cvcInputLayout->setSpacing(8);
    
    cvvInput = new QLineEdit(cvcContainer);
    cvvInput->setPlaceholderText("123");
    cvvInput->setMaxLength(4);
    cvvInput->setObjectName("cvcInput");
    cvvInput->setMinimumHeight(40);
    cvvInput->setFixedWidth(70);

    QLabel* cvcHelp = new QLabel(tr("<a href='#'>What is CVC?</a>"), cvcContainer);
    cvcHelp->setObjectName("helpLink");
    cvcHelp->setCursor(Qt::PointingHandCursor);
    connect(cvcHelp, &QLabel::linkActivated, this, &PaymentPage::showCVCInfo);

    cvcInputLayout->addWidget(cvvInput);
    cvcInputLayout->addWidget(cvcHelp);
    cvcInputLayout->addStretch();

    cvcLayout->addWidget(cvcLabel);
    cvcLayout->addLayout(cvcInputLayout);

    cardDetailsLayout->addWidget(expiryContainer);
    cardDetailsLayout->addWidget(cvcContainer);
    cardDetailsLayout->addStretch();

    paymentLayout->addWidget(cardDetailsContainer);
    paymentLayout->addSpacing(16);

    // Security Note
    QWidget* securityNote = new QWidget(paymentCard);
    QHBoxLayout* securityLayout = new QHBoxLayout(securityNote);
    securityLayout->setContentsMargins(0, 0, 0, 0);
    securityLayout->setSpacing(8);

    QLabel* lockIcon = new QLabel(securityNote);
    lockIcon->setPixmap(UIUtils::getIcon("lock", 16));
    lockIcon->setFixedSize(16, 16);
    
    QLabel* securityText = new QLabel(tr("Your credit card information is encrypted"), securityNote);
    securityText->setObjectName("securityText");

    securityLayout->addWidget(lockIcon);
    securityLayout->addWidget(securityText);
    securityLayout->addStretch();

    paymentLayout->addWidget(securityNote);

    // Add saved card option if applicable
    savedCardInfoContainer = new QWidget(paymentCard);
    savedCardInfoContainer->setObjectName("savedCardContainer");
    QVBoxLayout* savedCardLayout = new QVBoxLayout(savedCardInfoContainer);
    savedCardLayout->setContentsMargins(24, 16, 24, 16);
    savedCardLayout->setSpacing(8);

    useSavedCardCheckbox = new QCheckBox(tr("Use saved card"));
    useSavedCardCheckbox->setObjectName("useSavedCardCheckbox");
    connect(useSavedCardCheckbox, &QCheckBox::toggled, this, &PaymentPage::toggleSavedCardUsage);

    savedCardLabel = new QLabel();
    savedCardLabel->setObjectName("savedCardLabel");
    savedCardLabel->setWordWrap(true);

    savedCardLayout->addWidget(useSavedCardCheckbox);
    savedCardLayout->addWidget(savedCardLabel);
    savedCardInfoContainer->hide();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    backButton = new QPushButton(tr("Back"), paymentCard);
    backButton->setObjectName("backButton");
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setMinimumHeight(40);
    connect(backButton, &QPushButton::clicked, this, &PaymentPage::onBackButtonClicked);

    payButton = new QPushButton(tr("Pay Now"), paymentCard);
    payButton->setObjectName("payButton");
    payButton->setCursor(Qt::PointingHandCursor);
    payButton->setMinimumHeight(40);
    payButton->setIcon(UIUtils::getIcon("credit-card", 16));
    connect(payButton, &QPushButton::clicked, this, &PaymentPage::onPayButtonClicked);

    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(payButton);

    paymentCardLayout->addWidget(paymentSection);
    paymentCardLayout->addWidget(savedCardInfoContainer);
    paymentCardLayout->addLayout(buttonLayout);

    contentLayout->addWidget(paymentCard);

    // Apply modern shadows based on theme
    QGraphicsDropShadowEffect* cardShadow = new QGraphicsDropShadowEffect();
    cardShadow->setBlurRadius(20);
    cardShadow->setColor(QColor(0, 0, 0, isDarkTheme ? 80 : 30));
    cardShadow->setOffset(0, 2);
    paymentCard->setGraphicsEffect(cardShadow);

    // Set the content container as the scroll area's widget
    scrollArea->setWidget(contentContainer);
    
    // Add the scroll area to the main layout
    mainLayout->addWidget(scrollArea);
}

void PaymentPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    isUpdatingStyles = true;
    
    qDebug() << "PaymentPage::updateTheme called with isDark =" << isDark;
    
    // First, update the main background
    setStyleSheet(QString(
        "PaymentPage {"
        "   background-color: %1;"
        "}"
    ).arg(isDark ? "#111827" : "#F9FAFB"));

    // Get all container widgets that need color updates
    QList<QWidget*> allContainers = findChildren<QWidget*>();
    
    // ** DIRECTLY APPLY COLORS TO ALL CONTAINERS **
    foreach (QWidget* container, allContainers) {
        if (container->objectName() == "contentContainer") {
            container->setStyleSheet(QString(
                "QWidget#contentContainer {"
                "   background-color: %1;"
                "   border-radius: 16px;"
                "}"
            ).arg(isDark ? "#1E293B" : "#FFFFFF"));
        }
        else if (container->objectName() == "paymentCard") {
            container->setStyleSheet(QString(
                "QWidget#paymentCard {"
                "   background-color: %1;"
                "   border-radius: 16px;"
                "   border: 1px solid %2;"
                "}"
            ).arg(
                isDark ? "#1F2937" : "#FFFFFF",
                isDark ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"
            ));
        }
        else if (container->objectName() == "receiptSection") {
            container->setStyleSheet(QString(
                "QWidget#receiptSection {"
                "   background-color: %1;"
                "   border-radius: 16px;"
                "   border: 1px solid %2;"
                "}"
            ).arg(
                isDark ? "#1F2937" : "#FFFFFF",
                isDark ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"
            ));
        }
        else if (container->objectName() == "savedCardInfoContainer") {
            // Update the saved card container styling directly here
            container->setStyleSheet(QString(
                "QWidget#savedCardInfoContainer {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 12px;"
                "   padding: 12px;"
                "}"
            ).arg(
                isDark ? "rgba(30, 58, 138, 0.15)" : "rgba(219, 234, 254, 0.8)",
                isDark ? "rgba(59, 130, 246, 0.5)" : "rgba(37, 99, 235, 0.3)"
            ));
        }
    }

    QList<QLabel*> allLabels = findChildren<QLabel*>();
    foreach (QLabel* label, allLabels) {
        if (label->objectName() == "receiptLabel") {
            label->setStyleSheet(QString(
                "QLabel#receiptLabel {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 500;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        else if (label->objectName() == "receiptPrice") {
            label->setStyleSheet(QString(
                "QLabel#receiptPrice {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 600;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        else if (label->objectName() == "vipPrice") {
            label->setStyleSheet(QString(
                "QLabel#vipPrice {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 600;"
                "}"
            ).arg(isDark ? "#A78BFA" : "#8B5CF6"));
        }
        else if (label->objectName() == "totalLabel") {
            label->setStyleSheet(QString(
                "QLabel#totalLabel {"
                "   color: %1 !important;"
                "   font-size: 17px;"
                "   font-weight: 700;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        else if (label->objectName() == "totalPrice") {
            label->setStyleSheet(QString(
                "QLabel#totalPrice {"
                "   color: %1 !important;"
                "   font-size: 17px;"
                "   font-weight: 700;"
                "}"
            ).arg(isDark ? "#A78BFA" : "#8B5CF6"));
        }
        else if (label->objectName() == "savedCardLabel") {
            // Direct styling of saved card label
            label->setStyleSheet(QString(
                "QLabel#savedCardLabel {"
                "   color: %1;"
                "   font-size: 14px;"
                "   font-weight: 500;"
                "}"
            ).arg(isDark ? "#E2E8F0" : "#1E293B"));
        }
    }

    // Update scroll area styling
    QScrollArea* scrollArea = findChild<QScrollArea*>("paymentScrollArea");
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

    // Update title style
    updateTitleStyle();
    
    // Update all other component styles
    updateCardStyles();
    updateInputStyles();
    updateButtonStyles();

    QTimer::singleShot(0, this, [this]() {
        repaint();
        if (contentContainer) contentContainer->repaint();
        if (paymentCard) paymentCard->repaint();
        if (QWidget* receiptSection = findChild<QWidget*>("receiptSection")) {
            receiptSection->repaint();
        }
    });
    
    isUpdatingStyles = false;
    if (isLayoutUpdatePending) {
        isLayoutUpdatePending = false;
        updateLayout();
    }

    qDebug() << "PaymentPage::updateTheme completed";
}

void PaymentPage::updateCardStyles()
{
    if (isUpdatingStyles) return;
    isUpdatingStyles = true;

    // Apply modern shadows based on theme
    if (QGraphicsEffect* effect = paymentCard->graphicsEffect()) {
        QGraphicsDropShadowEffect* shadow = qobject_cast<QGraphicsDropShadowEffect*>(effect);
        if (shadow) {
            shadow->setColor(QColor(0, 0, 0, isDarkTheme ? 80 : 30));
        }
    }

    // Content container style with subtle gradient background
    contentContainer->setStyleSheet(QString(
        "QWidget#contentContainer {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "}"
    ).arg(isDarkTheme ? 
          "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #111827, stop:1 #1E293B)" : 
          "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #F9FAFB, stop:1 #F3F4F6)"));

    // Payment card style with refined borders and subtle transitions
    paymentCard->setStyleSheet(QString(
        "QWidget#paymentCard {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "   border: 1px solid %2;"
        "}"
    ).arg(
        isDarkTheme ? 
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1F2937, stop:1 #1A1F2C)" : 
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F9FAFB)",
        isDarkTheme ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"
    ));

    // Force update receipt section styles directly
    if (QWidget* receiptSection = findChild<QWidget*>("receiptSection")) {
        receiptSection->setStyleSheet(QString(
        "QWidget#receiptSection {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "   border: 1px solid %2;"
        "}"
        ).arg(
            isDarkTheme ? "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1F2937, stop:1 #1A1F2C)" : "#FFFFFF",
            isDarkTheme ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"
        ));
    }

    // Force update all receipt labels directly
    QList<QLabel*> receiptLabels = findChildren<QLabel*>("receiptLabel");
    for (QLabel* label : receiptLabels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
        "   font-weight: 500;"
            "   letter-spacing: 0.2px;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    // Force update receipt prices directly
    if (planPriceLabel) {
        planPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
        "   font-weight: 600;"
        "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    // Force update VIP price directly
    if (vipPriceLabel) {
        vipPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
        "   font-weight: 600;"
            "}"
        ).arg(isDarkTheme ? "#A78BFA" : "#8B5CF6"));
    }

    // Force update total label directly
    QList<QLabel*> totalLabels = findChildren<QLabel*>("totalLabel");
    for (QLabel* label : totalLabels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 17px;"
        "   font-weight: 700;"
            "   letter-spacing: 0.2px;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    // Force update total price directly
    if (totalPriceLabel) {
        totalPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 17px;"
        "   font-weight: 700;"
            "   letter-spacing: 0.2px;"
        "}"
        ).arg(isDarkTheme ? "#A78BFA" : "#8B5CF6"));
    }

    // Force update divider directly
    QList<QFrame*> dividers = findChildren<QFrame*>("divider");
    for (QFrame* divider : dividers) {
        divider->setStyleSheet(QString(
            "QFrame {"
            "   background: %1;"
        "   height: 1px;"
            "   margin: 16px 0;"
            "}"
        ).arg(isDarkTheme ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"));
    }

    // Field labels with refined typography - ensuring color contrast
    QList<QLabel*> fieldLabels = findChildren<QLabel*>("fieldLabel");
    for (QLabel* label : fieldLabels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
        "   font-size: 14px;"
        "   font-weight: 500;"
            "   margin-bottom: 6px;"
            "   letter-spacing: 0.2px;"
            "}"
        ).arg(isDarkTheme ? "#E2E8F0" : "#374151"));
    }

    // Help link with animated hover effect
    QList<QLabel*> helpLinks = findChildren<QLabel*>("helpLink");
    for (QLabel* link : helpLinks) {
        link->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 13px;"
            "   transition: color 0.2s ease;"
            "}"
            "QLabel:hover {"
            "   color: %2 !important;"
        "   text-decoration: underline;"
        "}"
        ).arg(
            isDarkTheme ? "#A78BFA" : "#7C3AED",
            isDarkTheme ? "#C4B5FD" : "#8B5CF6"
        ));
    }

    // Security text with subtle icon
    QList<QLabel*> securityTexts = findChildren<QLabel*>("securityText");
    for (QLabel* text : securityTexts) {
        text->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 13px;"
            "   letter-spacing: 0.2px;"
        "}"
        ).arg(isDarkTheme ? "#CBD5E1" : "#6B7280"));
    }

    // Input fields with refined aesthetics and interactive states 
    QString inputStyle = QString(
        "QLineEdit {"
        "   background-color: %1 !important;"
        "   color: %2 !important;"
        "   border: 1px solid %3 !important;"
        "   border-radius: 10px;"
        "   padding: 10px 14px;"
        "   font-size: 14px;"
        "   letter-spacing: 0.3px;"
        "   transition: all 0.2s ease;"
        "}"
        "QLineEdit:focus {"
        "   border-color: %4 !important;"
        "   background-color: %5 !important;"
        "   color: %2 !important;"
        "   box-shadow: 0 0 0 2px %8;"
        "}"
        "QLineEdit:hover:!focus {"
        "   border-color: %6 !important;"
        "   background-color: %7 !important;"
        "}"
        "QLineEdit::placeholder {"
        "   color: %9 !important;"
        "   letter-spacing: 0.2px;"
        "}"
    ).arg(
        isDarkTheme ? "#1F2937" : "#FFFFFF",  // Background color
        isDarkTheme ? "#FFFFFF" : "#111827",  // Text color - ensuring contrast
        isDarkTheme ? "#374151" : "#E5E7EB",  // Border color
        isDarkTheme ? "#8B5CF6" : "#7C3AED",  // Focus border color
        isDarkTheme ? "#2D3748" : "#F9FAFB",  // Focus background
        isDarkTheme ? "#4B5563" : "#D1D5DB",  // Hover border
        isDarkTheme ? "#2D3748" : "#F9FAFB",  // Hover background
        isDarkTheme ? "rgba(139, 92, 246, 0.2)" : "rgba(124, 58, 237, 0.12)", // Focus shadow
        isDarkTheme ? "#9CA3AF" : "#9CA3AF"   // Placeholder color
    );

    QList<QLineEdit*> inputs = findChildren<QLineEdit*>();
    for (QLineEdit* input : inputs) {
        input->setStyleSheet(inputStyle);
    }

    // Button styles - elegant back button with subtle hover effects
    backButton->setStyleSheet(QString(
        "QPushButton {"
        "   background-color: %1 !important;"
        "   color: %2 !important;"
        "   border: none !important;"
        "   border-radius: 10px;"
        "   padding: 10px 20px;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "   transition: all 0.2s ease;"
        "}"
        "QPushButton:hover {"
        "   background-color: %3 !important;"
        "   transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "   background-color: %4 !important;"
        "   transform: translateY(1px);"
        "}"
    ).arg(
        isDarkTheme ? "#374151" : "#EEF2FF",  // Normal background
        isDarkTheme ? "#FFFFFF" : "#4F46E5",  // Text color - high contrast
        isDarkTheme ? "#4B5563" : "#E0E7FF",  // Hover background
        isDarkTheme ? "#1F2937" : "#C7D2FE"   // Pressed background
    ));

    // Pay button with premium gradient and animation effects
    payButton->setStyleSheet(QString(
        "QPushButton {"
        "   background: %1 !important;"
        "   color: white !important;"
        "   border: none !important;"
        "   border-radius: 10px;"
        "   padding: 10px 24px;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "   letter-spacing: 0.3px;"
        "   transition: all 0.2s ease;"
        "}"
        "QPushButton:hover {"
        "   background: %2 !important;"
        "   transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "   background: %3 !important;"
        "   transform: translateY(1px);"
        "}"
    ).arg(
        isDarkTheme ? 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #6D28D9)" : 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #6D28D9)",
        isDarkTheme ? 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #A78BFA, stop:1 #7C3AED)" : 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #A78BFA, stop:1 #7C3AED)",
        isDarkTheme ? 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6D28D9, stop:1 #5B21B6)" : 
            "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6D28D9, stop:1 #5B21B6)"
    ));

    // Update page title with proper spacing
    titleLabel->setStyleSheet(QString(
        "QLabel#pageTitle {"
        "   color: %1 !important;"
        "   font-size: 22px;"
        "   font-weight: 700;"
        "   letter-spacing: -0.5px;"
        "   padding: 10px 0 !important;"
        "   margin: 0 !important;"
        "}"
    ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));

    isUpdatingStyles = false;
}

void PaymentPage::updateLayout()
{
    if (isUpdatingStyles || isLayoutUpdatePending) return;
    isLayoutUpdatePending = true;

    const QSize size = this->size();
    const qreal baseWidth = 1200.0;
    const qreal baseHeight = 800.0;
    const qreal minScale = 0.5;
    const qreal maxScale = 0.9;
    
    const qreal widthRatio = size.width() / baseWidth;
    const qreal heightRatio = size.height() / baseHeight;
    const qreal scale = qMin(qMin(widthRatio, heightRatio), maxScale);
    const qreal finalScale = qMax(scale, minScale);

    // Update font sizes
    int titleSize = qMax(20, static_cast<int>(28 * finalScale));
    int sectionTitleSize = qMax(16, static_cast<int>(18 * finalScale));
    int labelSize = qMax(12, static_cast<int>(14 * finalScale));
    int inputSize = qMax(12, static_cast<int>(14 * finalScale));

    // Update title
    titleLabel->setStyleSheet(QString(
        "QLabel {"
        "   color: %1;"
        "   font-size: %2px;"
        "   font-weight: 700;"
        "   letter-spacing: -0.5px;"
        "}"
    ).arg(
        isDarkTheme ? "#F9FAFB" : "#111827",
        QString::number(titleSize)
    ));

    // Update section titles
    QList<QLabel*> sectionTitles = findChildren<QLabel*>("sectionTitle");
    for (QLabel* title : sectionTitles) {
        title->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: %2px;"
            "   font-weight: 700;"
            "}"
        ).arg(
            isDarkTheme ? "#F9FAFB" : "#111827",
            QString::number(sectionTitleSize)
        ));
    }

    // Update labels
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1;"
            "   font-size: %2px;"
            "   font-weight: 500;"
            "}"
        ).arg(
            isDarkTheme ? "#E5E7EB" : "#4B5563",
            QString::number(labelSize)
        ));
    }

    // Update inputs
    QList<QLineEdit*> inputs = findChildren<QLineEdit*>();
    for (QLineEdit* input : inputs) {
        input->setStyleSheet(QString(
            "QLineEdit {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: 1px solid %3;"
            "   border-radius: 8px;"
            "   padding: 10px;"
            "   font-size: %4px;"
            "}"
        ).arg(
            isDarkTheme ? "#1F2937" : "#FFFFFF",
            isDarkTheme ? "#FFFFFF" : "#111827",
            isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)",
            QString::number(inputSize)
        ));
    }

    // Update buttons via updateCardStyles to ensure consistency
    updateCardStyles();

    isLayoutUpdatePending = false;
}

void PaymentPage::retranslateUI()
{
    if (isUpdatingStyles) return;
    isUpdatingStyles = true;
    
    qDebug() << "PaymentPage::retranslateUI called";

    // Title
    titleLabel->setText(tr("Payment Details"));
    
    // Button text
    backButton->setText(tr("Back"));
    payButton->setText(tr("Pay Now"));
    payButton->setIcon(UIUtils::getIcon("credit-card", 16));

    // Receipt section
    QList<QLabel*> receiptLabels = findChildren<QLabel*>("receiptLabel");
    foreach (QLabel* label, receiptLabels) {
        if (label->text().contains("Standard Plan")) {
            label->setText(tr("Standard Plan:"));
        } else if (label->text().contains("VIP Option")) {
            label->setText(tr("VIP Option:"));
        }
    }
    
    QList<QLabel*> totalLabels = findChildren<QLabel*>("totalLabel");
    foreach (QLabel* label, totalLabels) {
        label->setText(tr("TOTAL:"));
    }

    // Payment form labels
    QList<QLabel*> fieldLabels = findChildren<QLabel*>("fieldLabel");
    foreach (QLabel* label, fieldLabels) {
        if (label->text().contains("Card Number")) {
            label->setText(tr("Card Number"));
        } else if (label->text().contains("Name on Card")) {
            label->setText(tr("Name on Card"));
        } else if (label->text().contains("Expiry Date")) {
            label->setText(tr("Expiry Date"));
        } else if (label->text().contains("CVC")) {
            label->setText(tr("CVC"));
    }
    }

    // Placeholders
    if (cardNumberInput) cardNumberInput->setPlaceholderText(tr("1234 5678 9012 3456"));
    if (expiryDateInput) expiryDateInput->setPlaceholderText(tr("MM / YY"));
    if (cvvInput) cvvInput->setPlaceholderText(tr("123"));
    if (nameInput) nameInput->setPlaceholderText(tr("James Zapata"));

    // CVC help link
    QList<QLabel*> helpLinks = findChildren<QLabel*>("helpLink");
    foreach (QLabel* link, helpLinks) {
        link->setText(tr("<a href='#'>What is CVC?</a>"));
    }

    // Security text
    QList<QLabel*> securityTexts = findChildren<QLabel*>("securityText");
    foreach (QLabel* text, securityTexts) {
        text->setText(tr("Your credit card information is encrypted"));
    }

    // Saved card checkbox
    if (useSavedCardCheckbox) {
        useSavedCardCheckbox->setText(tr("Use saved card"));
    }
    
    // Update CVC dialog
    if (cvcDialog) {
        cvcDialog->setWindowTitle(tr("CVC Information"));
        cvcDialog->setText(tr("The CVC (Card Verification Code) is a 3 or 4 digit number found on the back of your card."));
    }

    isUpdatingStyles = false;
    updateLayout();
}

void PaymentPage::setPlanDetails(int planId, bool isVip)
{
    selectedPlanId = planId;
    isVipEnabled = isVip;

    // Calculate prices with more accurate values
    double basePrice, vipCost = 0.0;
    switch (planId) {
        case 0: // Monthly
            basePrice = 29.99;
            vipCost = 9.00;
            break;
        case 1: // 3 Months
            basePrice = 79.99;
            vipCost = 24.00;
            break;
        case 2: // 6 Months
            basePrice = 149.99;
            vipCost = 45.00;
            break;
        case 3: // Yearly
            basePrice = 249.99;
            vipCost = 75.00;
            break;
        default:
            return;
    }

    if (planPriceLabel) {
    planPriceLabel->setText(QString("$%1").arg(basePrice, 0, 'f', 2));
        planPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
            "   font-weight: 600;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }
    
    if (vipPriceLabel) {
    vipPriceLabel->setText(isVip ? QString("+$%1").arg(vipCost, 0, 'f', 2) : "$0.00");
        vipPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
            "   font-weight: 600;"
            "}"
        ).arg(isDarkTheme ? "#A78BFA" : "#8B5CF6"));
    }
    
    if (totalPriceLabel) {
    totalPriceLabel->setText(QString("$%1").arg(isVip ? basePrice + vipCost : basePrice, 0, 'f', 2));
        totalPriceLabel->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 17px;"
            "   font-weight: 700;"
            "   letter-spacing: 0.2px;"
            "}"
        ).arg(isDarkTheme ? "#A78BFA" : "#8B5CF6"));
    }

    // Ensure receipt labels have the correct color
    QList<QLabel*> receiptLabels = findChildren<QLabel*>("receiptLabel");
    foreach (QLabel* label, receiptLabels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 15px;"
            "   font-weight: 500;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    QList<QLabel*> totalLabels = findChildren<QLabel*>("totalLabel");
    foreach (QLabel* label, totalLabels) {
        label->setStyleSheet(QString(
            "QLabel {"
            "   color: %1 !important;"
            "   font-size: 17px;"
            "   font-weight: 700;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }

    if (QWidget* receiptSection = findChild<QWidget*>("receiptSection")) {
        receiptSection->update();
    }
}

void PaymentPage::onPayButtonClicked()
{
    if (validatePaymentDetails()) {
        processPayment();
    }
}

bool PaymentPage::validatePaymentDetails()
{
    QString errorMessage;
    bool isValid = true;

    // Check if using saved card
    bool usingSavedCard = useSavedCardCheckbox && useSavedCardCheckbox->isChecked() && hasSavedCard;
    
    if (usingSavedCard) {
        // If using saved card, only validate the CVV
        QString cvv = cvvInput ? cvvInput->text() : QString();
        if (cvv.length() < 3 || cvv.length() > 4 || !cvv.toInt()) {
            errorMessage += tr("Invalid CVV\n");
            isValid = false;
        } else {
            // Check if saved CVC is available and verify it matches
            if (!savedCardData.cvc.isEmpty() && cvv != savedCardData.cvc) {
                isValid = false;
                // Show a nicely styled error message
                QMessageBox errorDialog(this);
                errorDialog.setWindowTitle(tr("CVV Verification Failed"));
                
                // Get the current theme
                bool isDark = ThemeManager::getInstance().isDarkTheme();
                
                QString htmlMessage = QString(
                    "<div style='font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto; padding: 16px 0; max-width: 400px;'>"
                    "<h3 style='margin: 0 0 18px 0; color: %1; font-size: 20px; letter-spacing: -0.5px; font-weight: 700;'>Security Verification Failed</h3>"
                    "<p style='margin: 0 0 16px 0; color: %2; line-height: 1.6; font-size: 14px;'>"
                    "The CVV code you entered doesn't match the one associated with this saved card. "
                    "Please check and try again."
                    "</p>"
                    "<div style='margin: 20px 0; border-left: 3px solid %3; padding-left: 16px;'>"
                    "<p style='color: %2; line-height: 1.6; margin: 0; font-size: 14px;'>"
                    "For your security, you must enter the correct CVV code to use your saved card information."
                    "</p>"
                    "</div>"
                    "</div>"
                ).arg(
                    isDark ? "#FFFFFF" : "#111827",  // Title color
                    isDark ? "#E5E7EB" : "#374151",  // Main text color 
                    isDark ? "#EF4444" : "#DC2626"   // Error highlight color
                );
                
                errorDialog.setText(htmlMessage);
                errorDialog.setIcon(QMessageBox::Warning);
                errorDialog.setStyleSheet(QString(
                    "QMessageBox {"
                    "   background-color: %1;"
                    "   border-radius: 16px;"
                    "}"
                    "QMessageBox QLabel {"
                    "   background-color: transparent;"
                    "   padding: 0;"
                    "   margin: 0;"
                    "}"
                    "QPushButton {"
                    "   background-color: %2;"
                    "   color: white;"
                    "   border: none;"
                    "   border-radius: 8px;"
                    "   min-width: 100px;"
                    "   padding: 10px 16px;"
                    "   font-weight: 600;"
                    "   font-size: 14px;"
                    "}"
                    "QPushButton:hover {"
                    "   background-color: %3;"
                    "}"
                    "QPushButton:pressed {"
                    "   background-color: %4;"
                    "}"
                ).arg(
                    isDark ? "#1F2937" : "#FFFFFF",
                    isDark ? "#EF4444" : "#DC2626",  // Error button background
                    isDark ? "#DC2626" : "#B91C1C",  // Error button hover
                    isDark ? "#B91C1C" : "#991B1B"   // Error button pressed
                ));
                
                errorDialog.exec();
            }
        }
        
        // Re-validate expiry date
        QString expiry = savedCardData.expiryDate;
        QRegularExpression expiryRegex("^(0[1-9]|1[0-2]) / \\d{2}$");
        if (!expiryRegex.match(expiry).hasMatch()) {
            errorMessage += tr("Invalid saved card expiry date\n");
            isValid = false;
        } else {
            // Check if card is expired
            int month = expiry.left(2).toInt();
            int year = 2000 + expiry.right(2).toInt();
            QDate expiryDate(year, month, 1);
            if (expiryDate <= QDate::currentDate()) {
                errorMessage += tr("Your saved card has expired. Please use a different card.\n");
                isValid = false;
            }
        }
    } else {
        // Validate card number (16 digits)
        QString cardNumber = cardNumberInput ? cardNumberInput->text().remove(' ') : QString();
        if (cardNumber.length() != 16 || !cardNumber.toULongLong()) {
            errorMessage += tr("Invalid card number\n");
            isValid = false;
        }

        // Validate expiry date
        QString expiry = expiryDateInput ? expiryDateInput->text() : QString();
        QRegularExpression expiryRegex("^(0[1-9]|1[0-2]) / \\d{2}$");
        if (!expiryRegex.match(expiry).hasMatch()) {
            errorMessage += tr("Invalid expiry date\n");
            isValid = false;
        } else {
            // Check if card is expired
            int month = expiry.left(2).toInt();
            int year = 2000 + expiry.right(2).toInt();
            QDate expiryDate(year, month, 1);
            if (expiryDate <= QDate::currentDate()) {
                errorMessage += tr("Card has expired\n");
                isValid = false;
            }
        }

        // Validate CVV (3-4 digits)
        QString cvv = cvvInput ? cvvInput->text() : QString();
        if (cvv.length() < 3 || cvv.length() > 4 || !cvv.toInt()) {
            errorMessage += tr("Invalid CVV\n");
            isValid = false;
        }

        // Validate name
        QString name = nameInput ? nameInput->text().trimmed() : QString();
        if (name.isEmpty() || !name.contains(' ')) {
            errorMessage += tr("Please enter your full name\n");
            isValid = false;
        }
    }

    if (!isValid && !errorMessage.isEmpty()) {
        QMessageBox::warning(this, tr("Invalid Payment Details"), errorMessage);
    }

    return isValid;
}

void PaymentPage::processPayment()
{
    if (!validatePaymentDetails()) {
        return;
    }

    // Create a non-modal dialog with QProgressDialog
    QProgressDialog* processingDialog = new QProgressDialog(tr("Processing payment..."), tr("Cancel"), 0, 0, this);
    processingDialog->setWindowModality(Qt::WindowModal);
    processingDialog->setWindowTitle(tr("Payment Processing"));
    processingDialog->setCancelButton(nullptr); // Disable cancel button
    processingDialog->setMinimumDuration(0); // Show immediately
    processingDialog->setAutoClose(false);
    processingDialog->setAutoReset(false);
    processingDialog->setFixedSize(300, 100);
    
    processingDialog->setStyleSheet(QString(
        "QProgressDialog {"
        "   background-color: %1;"
        "   border-radius: 12px;"
        "}"
        "QProgressDialog QLabel {"
        "   color: %2;"
        "   font-size: 14px;"
        "   margin: 10px;"
        "   font-weight: 500;"
        "}"
        "QProgressBar {"
        "   border: none;"
        "   border-radius: 6px;"
        "   background: %3;"
        "   height: 12px;"
        "   margin: 10px;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: %4;"
        "   border-radius: 6px;"
        "}"
    ).arg(
        isDarkTheme ? "#1F2937" : "#FFFFFF",
        isDarkTheme ? "#FFFFFF" : "#111827",
        isDarkTheme ? "#374151" : "#E5E7EB",
        isDarkTheme ? "#8B5CF6" : "#7C3AED"
    ));
    
    // Show processing dialog
    processingDialog->show();
    
    // Process payment after a short delay to allow dialog to appear
    QTimer::singleShot(100, this, [this, processingDialog]() {
        bool success = false;
        QString errorMessage;
        
        try {
            // Simulate processing delay
            QThread::msleep(500);
            
            // Process payment
            qDebug() << "Creating subscription for user ID: " << currentUserId;
            success = createSubscription();

            bool usingSavedCard = useSavedCardCheckbox && useSavedCardCheckbox->isChecked() && hasSavedCard;
            
            if (success && currentMemberId > 0) {
                bool cardSaveSuccess = false;
                
                if (usingSavedCard) {
                    cardSaveSuccess = true;
                    qDebug() << "Using existing saved card for transaction";
                } else {
                    // Save the newly entered card data
                    cardSaveSuccess = memberManager->saveCardData(
                        currentMemberId,
                        cardNumberInput->text().remove(' '),
                        expiryDateInput->text(),
                        nameInput->text().trimmed(),
                        cvvInput->text(),  // Save CVC for verification
                        errorMessage
                    );
                }
                
                if (!cardSaveSuccess) {
                    qDebug() << "Failed to store card data in memory: " << errorMessage;
                }
            }
        }
        catch (const std::exception& e) {
            qDebug() << "Exception during payment processing: " << e.what();
            errorMessage = e.what();
            success = false;
        }
        catch (...) {
            qDebug() << "Unknown exception during payment processing";
            errorMessage = "Unknown error";
            success = false;
        }
        
        // Close dialog
        processingDialog->close();
        processingDialog->deleteLater();
        
        // Process results
        if (!success) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create subscription: %1").arg(
                errorMessage.isEmpty() ? tr("Please try again.") : errorMessage));
        } else {
            // Create a beautiful success message
            QMessageBox successDialog(this);
            successDialog.setWindowTitle(tr("Payment Successful"));
            successDialog.setIcon(QMessageBox::NoIcon);
            
            // Create rich HTML content for the message
            QString successMessage = QString(
                "<div style='text-align: center; padding: 10px; font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto;'>"
                "<div style='margin-bottom: 20px;'>"
                "<span style='font-size: 48px; color: #4CAF50;'>✓</span>"
                "</div>"
                "<h2 style='margin: 0 0 15px 0; color: %1; font-size: 22px; font-weight: 600;'>"
                "Payment Successful!"
                "</h2>"
                "<p style='margin: 0 0 20px 0; color: %2; line-height: 1.5; font-size: 14px;'>"
                "Your subscription has been activated successfully.<br>"
                "Welcome to the FitFlex Pro family!"
                "</p>"
                "<div style='padding: 15px; background-color: %3; border-radius: 8px; margin-bottom: 15px;'>"
                "<p style='margin: 0; color: %4; font-size: 13px; text-align: left;'>"
                "<strong>💡 Tip:</strong> Your membership details and payment information<br>"
                "will be automatically saved when you exit the application."
                "</p>"
                "</div>"
                "</div>"
            ).arg(
                isDarkTheme ? "#FFFFFF" : "#111827",  // Title color
                isDarkTheme ? "#E5E7EB" : "#374151",  // Text color
                isDarkTheme ? "#1F2937" : "#F3F4F6",  // Background color for tip
                isDarkTheme ? "#9CA3AF" : "#4B5563"   // Tip text color
            );
            
            successDialog.setText(successMessage);
            
            // Style the dialog
            successDialog.setStyleSheet(QString(
                "QMessageBox {"
                "    background-color: %1;"
                "    border-radius: 16px;"
                "}"
                "QMessageBox QLabel {"
                "    background-color: transparent;"
                "}"
                "QPushButton {"
                "    background-color: %2;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 8px 16px;"
                "    font-weight: 600;"
                "    font-size: 14px;"
                "    min-width: 100px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %3;"
                "}"
            ).arg(
                isDarkTheme ? "#1F2937" : "#FFFFFF",
                isDarkTheme ? "#8B5CF6" : "#7C3AED",
                isDarkTheme ? "#7C3AED" : "#6D28D9"
            ));
            
            // Show the dialog and wait for user to close it
            successDialog.exec();
            
            // Emit completion signal
            emit paymentCompleted(selectedPlanId, isVipEnabled);
        }
    });
}

bool PaymentPage::createSubscription()
{
    if (!memberManager) {
        qDebug() << "Member manager is null";
        return false;
    }

    if (!userDataManager) {
        qDebug() << "User data manager is null";
        return false;
    }

    // Check if we already have the user ID
    int userId = currentUserId;
    qDebug() << "Initial userId from PaymentPage: " << userId;
    if (userId <= 0) {
        QString email;
        QString password;
        bool hasRememberedCreds = userDataManager->getRememberedCredentials(email, password);
        
        if (hasRememberedCreds) {
            // Get the user ID
            User user = userDataManager->getUserData(email);
            userId = user.getId();
            qDebug() << "Found user ID from remembered credentials: " << userId << " for email: " << email;
        }
    }
    
    if (userId <= 0) {
        qDebug() << "Could not determine user ID for subscription creation";
        return false;
    }
    qDebug() << "Creating subscription for user ID: " << userId;
    
    // Check if the user is already a member
    bool isMember = false;
    try {
        isMember = memberManager->userIsMember(userId);
        qDebug() << "User is already a member: " << isMember;
    } catch (const std::exception& e) {
        qDebug() << "Exception checking member status: " << e.what();
    } catch (...) {
        qDebug() << "Unknown exception checking member status";
    }
    
    if (!isMember) {
        qDebug() << "Creating new member for user ID: " << userId;
        QString errorMessage;
        
        try {
            User user = userDataManager->getUserDataById(userId);
            if (user.getId() <= 0) {
                qDebug() << "Could not find user with ID: " << userId;
                return false;
            }
            
            qDebug() << "Found user data: " << user.getName() << ", " << user.getEmail();
            
            if (memberManager->createMemberFromUser(user, errorMessage)) {
                qDebug() << "Successfully created member from user (data will be saved on exit)";
                currentMemberId = memberManager->getMemberIdByUserId(userId);
                qDebug() << "New member ID: " << currentMemberId;
            } else {
                qDebug() << "Failed to create member: " << errorMessage;
                return false;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception creating member: " << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception creating member";
            return false;
        }
    } else {
        // User is already a member, get their member ID
        currentMemberId = memberManager->getMemberIdByUserId(userId);
        qDebug() << "Retrieved existing member ID: " << currentMemberId << " for user ID: " << userId;
    }

    if (currentMemberId <= 0) {
        qDebug() << "Invalid or missing member ID: " << currentMemberId;
        return false;
    }

    // Get member data
    QString errorMessage;
    Member member;
    
    try {
        member = memberManager->getMemberById(currentMemberId);
        if (member.getId() == 0) {
            qDebug() << "Member not found with ID: " << currentMemberId;
            return false;
        }
        qDebug() << "Retrieved member record: " << member.toString();
    } catch (const std::exception& e) {
        qDebug() << "Exception getting member data: " << e.what();
        return false;
    } catch (...) {
        qDebug() << "Unknown exception getting member data";
        return false;
    }

    // Determine subscription type
    SubscriptionType type;
    switch (selectedPlanId) {
        case 0: type = SubscriptionType::MONTHLY; break;
        case 1: type = SubscriptionType::THREE_MONTHS; break;
        case 2: type = SubscriptionType::SIX_MONTHS; break;
        case 3: type = SubscriptionType::YEARLY; break;
        default: 
            qDebug() << "Invalid plan ID: " << selectedPlanId;
            return false;
    }

    try {
        // Create new subscription with a fixed current date to avoid system date issues
        QDate currentYear = QDate::currentDate();
        // Check if the year is reasonable (between 2023 and 2026)
        if (currentYear.year() < 2023 || currentYear.year() > 2026) {
            qDebug() << "WARNING: System date appears incorrect: " << currentYear.toString() 
                    << " - Using explicit current date instead";
            // Force date to be a reasonable current date
            currentYear = QDate(2023, 11, 15); // Use a fixed date that's definitely valid
        }
        
        // Log the actual date we're using
        qDebug() << "Using subscription start date:" << currentYear.toString(Qt::ISODate);
        
        Subscription subscription(type, currentYear);
        subscription.setVIP(isVipEnabled);
        
        qDebug() << "Created subscription: Type=" << (int)type 
                 << ", Start=" << currentYear.toString()
                 << ", VIP=" << isVipEnabled;

        // Add subscription to member
        member.setSubscription(subscription);

        // Update member in data manager (will be saved on app exit)
        bool updateSuccess = memberManager->updateMember(member, errorMessage);
        if (!updateSuccess) {
            qDebug() << "Failed to update member data: " << errorMessage;
            return false;
        }
        
        qDebug() << "Subscription created successfully for member ID: " << currentMemberId << " (data will be saved on exit)";
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Exception setting subscription: " << e.what();
        return false;
    } catch (...) {
        qDebug() << "Unknown exception setting subscription";
        return false;
    }
}

void PaymentPage::setCurrentMemberId(int memberId)
{
    currentMemberId = memberId;
    loadSavedCardData();
}

void PaymentPage::loadSavedCardData()
{
    // Reset saved card flag
    hasSavedCard = false;
    
    // Check if we have valid member info
    if (!memberManager || currentMemberId <= 0) {
        savedCardInfoContainer->hide();
        return;
    }
    
    // Check if member has saved card
    hasSavedCard = memberManager->hasStoredCard(currentMemberId);
    if (hasSavedCard) {
        // Retrieve saved card
        savedCardData = memberManager->getStoredCard(currentMemberId);
        
        // Format card info for display
        QString lastFour = savedCardData.fullCardNumber.right(4);
        QString maskedNumber = QString("**** **** **** ").append(lastFour);
        
        // Get the layout from the container
        QVBoxLayout* savedCardLayout = qobject_cast<QVBoxLayout*>(savedCardInfoContainer->layout());
        
        // Clear existing content (important to prevent duplication during theme change)
        if (savedCardLayout) {
            // Remove all widgets except the checkbox
            QLayoutItem* item;
            while ((item = savedCardLayout->takeAt(1)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
        }
        
        // Create a new container widget for card info
        QWidget* savedCardContentWidget = new QWidget(savedCardInfoContainer);
        savedCardContentWidget->setObjectName("savedCardContent");
        QHBoxLayout* savedCardContentLayout = new QHBoxLayout(savedCardContentWidget);
        savedCardContentLayout->setContentsMargins(0, 8, 0, 8);
        savedCardContentLayout->setSpacing(12);
        
        // Add credit card icon
        QLabel* cardIconLabel = new QLabel(savedCardContentWidget);
        cardIconLabel->setObjectName("cardIconLabel");
        QString cardIconColor = isDarkTheme ? "#60A5FA" : "#2563EB";
        QPixmap cardIcon = UIUtils::getIconWithColor("credit-card", QColor(cardIconColor), 32);
        cardIconLabel->setPixmap(cardIcon);
        cardIconLabel->setFixedSize(32, 32);
        
        // Create a new label for card info
        QLabel* cardInfoLabel = new QLabel(savedCardContentWidget);
        cardInfoLabel->setObjectName("savedCardLabel");
        cardInfoLabel->setWordWrap(true);
        
        // Set formatted card info on the label
        QString cardInfo = tr("Card ending in %1\nCardholder: %2\nExpiry: %3")
            .arg(lastFour)
            .arg(savedCardData.cardholderName)
            .arg(savedCardData.expiryDate);
        cardInfoLabel->setText(cardInfo);
        
        // Add icon and info to layout
        savedCardContentLayout->addWidget(cardIconLabel);
        savedCardContentLayout->addWidget(cardInfoLabel, 1);
        
        // Add the content widget to the main layout
        savedCardLayout->addWidget(savedCardContentWidget);
        
        // Update the reference to the saved card label
        savedCardLabel = cardInfoLabel;
        
        // Show the saved card container and update its style
        savedCardInfoContainer->show();
        useSavedCardCheckbox->setChecked(false);  // Unchecked by default
        
        // Update the styles
        updateInputStyles();
    } else {
        // No saved card, hide the container
        savedCardInfoContainer->hide();
    }
}

void PaymentPage::toggleSavedCardUsage(bool checked)
{
    if (checked && hasSavedCard) {
        // Populate the form with saved card data
        populateFieldsWithSavedCard();
        
        // Disable the card fields but enable the CVV field for security
        cardNumberInput->setEnabled(false);
        nameInput->setEnabled(false);
        expiryDateInput->setEnabled(false);
        cvvInput->setEnabled(true);
        cvvInput->setFocus(); // Set focus to the CVV field
    } else {
        // Enable all fields
        cardNumberInput->setEnabled(true);
        nameInput->setEnabled(true);
        expiryDateInput->setEnabled(true);
        cvvInput->setEnabled(true);
        
        // Clear fields if user unchecks the box
        if (!checked) {
            cardNumberInput->clear();
            nameInput->clear();
            expiryDateInput->clear();
            cvvInput->clear();
        }
    }
}

void PaymentPage::populateFieldsWithSavedCard()
{
    // Populate fields with saved card data
    cardNumberInput->setText(savedCardData.fullCardNumber);
    nameInput->setText(savedCardData.cardholderName);
    expiryDateInput->setText(savedCardData.expiryDate);
    cvvInput->clear(); // Always clear CVV for security
}

void PaymentPage::onBackButtonClicked()
{
    emit backToSubscription();
}

void PaymentPage::resizeEvent(QResizeEvent* event)
{
        QWidget::resizeEvent(event);
    
    // Force title label to have minimal height
    if (titleLabel) {
        titleLabel->setMaximumHeight(titleLabel->sizeHint().height());
    }
    
    // Proceed with normal layout update
    if (!isUpdatingStyles && !isLayoutUpdatePending) {
    updateLayout();
    }
}

void PaymentPage::setupCVCDialog()
{
    if (!cvcDialog) {
        cvcDialog = new QMessageBox(this);
    }
    
    // Get the current theme state directly from ThemeManager
    bool currentIsDarkTheme = ThemeManager::getInstance().isDarkTheme();
    
    cvcDialog->setWindowTitle(tr("CVC Information"));
    cvcDialog->setIcon(QMessageBox::Information);
    
    QString infoText = tr(
        "<div style='font-family: -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto; padding: 16px 0; max-width: 400px;'>"
        "<h3 style='margin: 0 0 18px 0; color: %1; font-size: 20px; letter-spacing: -0.5px; font-weight: 700;'>What is CVC?</h3>"
        "<p style='margin: 0 0 16px 0; color: %2; line-height: 1.6; font-size: 14px;'>"
        "The CVC (Card Verification Code) is a security feature that helps protect you against fraud:"
        "</p>"
        "<div style='margin: 20px 0; border-left: 3px solid %3; padding-left: 16px;'>"
        "<ul style='color: %2; line-height: 1.7; margin: 0; padding-left: 12px; font-size: 14px;'>"
        "<li style='margin-bottom: 10px;'>For Visa, MasterCard, and Discover cards:<br>"
        "<span style='color: %3; font-weight: 600;'>3 digits on the back of your card</span></li>"
        "<li>For American Express cards:<br>"
        "<span style='color: %3; font-weight: 600;'>4 digits on the front of your card</span></li>"
        "</ul>"
        "</div>"
        "<p style='margin: 16px 0 0 0; color: %4; font-size: 13px; line-height: 1.5;'>"
        "💡 Never share your CVC with anyone except when making secure purchases."
        "</p>"
        "</div>"
    ).arg(
        currentIsDarkTheme ? "#FFFFFF" : "#111827",  // Title color
        currentIsDarkTheme ? "#E5E7EB" : "#374151",  // Main text color
        currentIsDarkTheme ? "#8B5CF6" : "#7C3AED",  // Highlight color
        currentIsDarkTheme ? "#9CA3AF" : "#6B7280"   // Note color
    );

    cvcDialog->setText(infoText);
    cvcDialog->setStyleSheet(QString(
        "QMessageBox {"
        "   background-color: %1;"
        "   border-radius: 16px;"
        "}"
        "QMessageBox QLabel {"
        "   background-color: transparent;"
        "   padding: 0;"
        "   margin: 0;"
        "}"
        "QPushButton {"
        "   background-color: %5;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   min-width: 100px;"
        "   padding: 10px 16px;"
        "   font-weight: 600;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %6;"
        "}"
        "QPushButton:pressed {"
        "   background-color: %7;"
        "}"
    ).arg(
        currentIsDarkTheme ? "#1F2937" : "#FFFFFF",
        currentIsDarkTheme ? "#374151" : "#F3F4F6",
        currentIsDarkTheme ? "#FFFFFF" : "#111827",
        currentIsDarkTheme ? "#4B5563" : "#E5E7EB",
        currentIsDarkTheme ? "#8B5CF6" : "#7C3AED",  // Button background
        currentIsDarkTheme ? "#7C3AED" : "#6D28D9",  // Button hover
        currentIsDarkTheme ? "#6D28D9" : "#5B21B6"   // Button pressed
    ));
}

void PaymentPage::showCVCInfo()
{
    setupCVCDialog();
    cvcDialog->exec();
}

void PaymentPage::validateExpiryDate(const QString& text)
{
    if (text.isEmpty()) {
        return;
    }

    QString filtered = text;
    filtered.remove(QRegularExpression("[^0-9]"));
    
    QString formatted;
    int month = 0;
    
    // Handle month input (first two digits)
    if (filtered.length() >= 2) {
        month = filtered.left(2).toInt();
        if (month > 12) {
            month = 12;
            filtered = "12" + filtered.mid(2);
        } else if (month == 0) {
            month = 1;
            filtered = "01" + filtered.mid(2);
        } else if (month < 10 && !filtered.startsWith('0')) {
            filtered = "0" + QString::number(month) + filtered.mid(2);
        }
    }
    
    // Format the date with separator
    for (int i = 0; i < filtered.length() && i < 4; ++i) {
        if (i == 2) {
            formatted += " / ";
        }
        formatted += filtered[i];
    }
    
    // Update the text if it's different
    QLineEdit* expiryInput = qobject_cast<QLineEdit*>(sender());
    if (!expiryInput) {
        return;
    }

    if (formatted != text) {
        expiryInput->setText(formatted);
        expiryInput->setCursorPosition(formatted.length());
    }
    
    // Validate the expiry date
    if (filtered.length() == 4) {
        int year = 2000 + filtered.right(2).toInt();
        QDate currentDate = QDate::currentDate();
        QDate expiryDate(year, month, 1);
        
        if (expiryDate < currentDate) {
            expiryInput->setStyleSheet(expiryInput->styleSheet() + 
                "QLineEdit { border-color: #EF4444 !important; }");
        } else {
            updateCardStyles();  // Reset to normal style
        }
    }
}

void PaymentPage::updateInputStyles()
{
    // Get all input fields
    QList<QLineEdit*> inputs = findChildren<QLineEdit*>();
    QList<QLabel*> fieldLabels = findChildren<QLabel*>("fieldLabel");
    QList<QLabel*> helpLinks = findChildren<QLabel*>("helpLink");
    QList<QLabel*> securityTexts = findChildren<QLabel*>("securityText");
    
    // Field labels
    foreach (QLabel* label, fieldLabels) {
        label->setStyleSheet(QString(
            "QLabel#fieldLabel {"
            "   color: %1;"
            "   font-size: 14px;"
            "   font-weight: 600;"
            "   margin-bottom: 4px;"
            "}"
        ).arg(isDarkTheme ? "#E5E7EB" : "#374151"));
    }
    
    // Help links
    foreach (QLabel* link, helpLinks) {
        link->setStyleSheet(QString(
            "QLabel#helpLink {"
            "   color: %1;"
            "   text-decoration: none;"
            "   font-size: 13px;"
            "}"
        ).arg(isDarkTheme ? "#60A5FA" : "#3B82F6"));
    }
    
    // Security text
    foreach (QLabel* text, securityTexts) {
        text->setStyleSheet(QString(
            "QLabel#securityText {"
            "   color: %1;"
            "   font-size: 13px;"
            "}"
        ).arg(isDarkTheme ? "#9CA3AF" : "#6B7280"));
    }
    
    // Line edits
    foreach (QLineEdit* input, inputs) {
        input->setStyleSheet(QString(
            "QLineEdit {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: 1px solid %3;"
            "   border-radius: 10px;"
            "   padding: 8px 12px;"
            "   font-size: 15px;"
            "}"
            "QLineEdit:focus {"
            "   border: 1px solid %4;"
            "   background-color: %5;"
            "}"
            "QLineEdit:hover:!focus {"
            "   border: 1px solid %6;"
            "}"
            "QLineEdit:disabled {"
            "   background-color: %7;"
            "   color: %8;"
            "}"
        ).arg(
            isDarkTheme ? "#1F2937" : "#FFFFFF",
            isDarkTheme ? "#F3F4F6" : "#1F2937",
            isDarkTheme ? "#4B5563" : "#D1D5DB",
            isDarkTheme ? "#60A5FA" : "#3B82F6",
            isDarkTheme ? "#374151" : "#F9FAFB",
            isDarkTheme ? "#6B7280" : "#9CA3AF",
            isDarkTheme ? "#111827" : "#F3F4F6",
            isDarkTheme ? "#9CA3AF" : "#6B7280"
        ));
    }
    
    // Apply styles to the checkbox
    if (useSavedCardCheckbox) {
        useSavedCardCheckbox->setStyleSheet(QString(
            "QCheckBox {"
            "   color: %1;"
            "   font-size: 16px;"
            "   font-weight: 600;"
            "   spacing: 8px;"
            "   padding: 4px;"
            "}"
            "QCheckBox::indicator {"
            "   width: 22px;"
            "   height: 22px;"
            "   border: 2px solid %2;"
            "   border-radius: 6px;"
            "   background: %3;"
            "}"
            "QCheckBox::indicator:checked {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3B82F6, stop:1 #2563EB);"
            "   border: 2px solid #3B82F6;"
            "   image: url(:/Images/check.png);"
            "}"
            "QCheckBox::indicator:hover {"
            "   border: 2px solid #3B82F6;"
            "   background-color: rgba(59, 130, 246, 0.1);"
            "}"
        ).arg(
            isDarkTheme ? "#FFFFFF" : "#1E293B",
            isDarkTheme ? "#4B5563" : "#D1D5DB",
            isDarkTheme ? "#1F2937" : "#FFFFFF"
        ));
    }
    
    // Update card icon if it exists
    QLabel* cardIconLabel = findChild<QLabel*>("cardIconLabel");
    if (cardIconLabel) {
        QString cardIconColor = isDarkTheme ? "#60A5FA" : "#2563EB";
        QPixmap cardIcon = UIUtils::getIconWithColor("credit-card", QColor(cardIconColor), 32);
        cardIconLabel->setPixmap(cardIcon);
    }
    
    // Saved card container styling
    if (savedCardInfoContainer && hasSavedCard) {
        QString bgColor = isDarkTheme ? "rgba(30, 58, 138, 0.15)" : "rgba(219, 234, 254, 0.8)";
        QString borderColor = isDarkTheme ? "rgba(59, 130, 246, 0.5)" : "rgba(37, 99, 235, 0.3)";
        QString textColor = isDarkTheme ? "#E2E8F0" : "#1E293B";
        
        savedCardInfoContainer->setStyleSheet(QString(R"(
            QWidget#savedCardContainer {
                background-color: %1;
                border: 1px solid %2;
                border-radius: 12px;
                padding: 12px;
                margin: 16px 0;
            }
            QLabel#savedCardLabel {
                color: %3;
                font-size: 15px;
                margin-left: 24px;
                line-height: 1.5;
            }
        )").arg(bgColor, borderColor, textColor));
    }
}

void PaymentPage::updateTitleStyle()
{
    if (titleLabel) {
        titleLabel->setStyleSheet(QString(
            "QLabel#pageTitle {"
            "   color: %1 !important;"
            "   font-size: 22px;"
            "   font-weight: 700;"
            "   letter-spacing: -0.5px;"
            "   padding: 10px 0 !important;"
            "   margin: 0 !important;"
            "}"
        ).arg(isDarkTheme ? "#FFFFFF" : "#111827"));
    }
}

void PaymentPage::updateButtonStyles()
{
    // Style for the Pay button (primary action)
    if (payButton) {
        payButton->setStyleSheet(QString(
            "QPushButton#payButton {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #3B82F6, stop:1 #2563EB);"
            "   color: white;"
            "   border: none;"
            "   border-radius: 12px;"
            "   padding: 12px 24px;"
            "   font-size: 16px;"
            "   font-weight: 700;"
            "   min-width: 120px;"
            "}"
            "QPushButton#payButton:hover {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #2563EB, stop:1 #1D4ED8);"
            "}"
            "QPushButton#payButton:pressed {"
            "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "       stop:0 #1D4ED8, stop:1 #1E40AF);"
            "}"
        ));
    }
    
    // Style for the Back button (secondary action)
    if (backButton) {
        backButton->setStyleSheet(QString(
            "QPushButton#backButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: 1px solid %3;"
            "   border-radius: 12px;"
            "   padding: 12px 24px;"
            "   font-size: 16px;"
            "   font-weight: 600;"
            "   min-width: 100px;"
            "}"
            "QPushButton#backButton:hover {"
            "   background-color: %4;"
            "   border-color: %5;"
            "}"
            "QPushButton#backButton:pressed {"
            "   background-color: %6;"
            "}"
        ).arg(
            isDarkTheme ? "#1F2937" : "#F9FAFB",
            isDarkTheme ? "#F9FAFB" : "#111827",
            isDarkTheme ? "#374151" : "#D1D5DB",
            isDarkTheme ? "#374151" : "#F3F4F6", 
            isDarkTheme ? "#4B5563" : "#9CA3AF",
            isDarkTheme ? "#111827" : "#E5E7EB"
        ));
    }
}

// Add this method after retranslateUI()
void PaymentPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "Language change event received in PaymentPage";
        retranslateUI();
    }
    else if (event->type() == QEvent::ApplicationPaletteChange || event->type() == QEvent::PaletteChange) {
        qDebug() << "Palette change event received in PaymentPage - simple update";
        
        // Avoid recursive calls
        if (isUpdatingStyles) {
            QWidget::changeEvent(event);
            return;
        }
        
        isUpdatingStyles = true;
        
        // Get theme state
        bool isDark = ThemeManager::getInstance().isDarkTheme();
        isDarkTheme = isDark;

        if (QWidget* receiptSection = findChild<QWidget*>("receiptSection")) {
            receiptSection->setStyleSheet(QString(
                "QWidget#receiptSection {"
                "   background-color: %1;"
                "   border-radius: 16px;"
                "   border: 1px solid %2;"
                "}"
            ).arg(
                isDark ? "#1F2937" : "#FFFFFF",
                isDark ? "rgba(255, 255, 255, 0.08)" : "rgba(0, 0, 0, 0.05)"
            ));
        }
        
        // 2. Update text colors (important labels only)
        QList<QLabel*> receiptLabels = findChildren<QLabel*>("receiptLabel");
        foreach (QLabel* label, receiptLabels) {
            label->setStyleSheet(QString(
                "QLabel#receiptLabel {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 500;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        
        // 3. Update prices directly 
        if (planPriceLabel) {
            planPriceLabel->setStyleSheet(QString(
                "QLabel#receiptPrice {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 600;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        
        if (vipPriceLabel) {
            vipPriceLabel->setStyleSheet(QString(
                "QLabel#vipPrice {"
                "   color: %1 !important;"
                "   font-size: 15px;"
                "   font-weight: 600;"
                "}"
            ).arg(isDark ? "#A78BFA" : "#8B5CF6"));
        }
        
        if (totalPriceLabel) {
            totalPriceLabel->setStyleSheet(QString(
                "QLabel#totalPrice {"
                "   color: %1 !important;"
                "   font-size: 17px;"
                "   font-weight: 700;"
                "}"
            ).arg(isDark ? "#A78BFA" : "#8B5CF6"));
        }
        
        // 4. Update total label
        QList<QLabel*> totalLabels = findChildren<QLabel*>("totalLabel");
        foreach (QLabel* label, totalLabels) {
            label->setStyleSheet(QString(
                "QLabel#totalLabel {"
                "   color: %1 !important;"
                "   font-size: 17px;"
                "   font-weight: 700;"
                "}"
            ).arg(isDark ? "#FFFFFF" : "#111827"));
        }
        if (savedCardInfoContainer) {
            savedCardInfoContainer->setStyleSheet(QString(
                "QWidget {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 12px;"
                "}"
            ).arg(
                isDark ? "rgba(30, 58, 138, 0.15)" : "rgba(219, 234, 254, 0.8)",
                isDark ? "rgba(59, 130, 246, 0.5)" : "rgba(37, 99, 235, 0.3)"
            ));
        }
        
        // 6. Don't rebuild the saved card display, just update the text color
        if (savedCardLabel) {
            savedCardLabel->setStyleSheet(QString(
                "QLabel {"
                "   color: %1;"
                "   font-size: 14px;"
                "   font-weight: 500;"
                "}"
            ).arg(isDark ? "#E2E8F0" : "#1E293B"));
        }
        
        // 7. Update checkbox styling without recreating it
        if (useSavedCardCheckbox) {
            useSavedCardCheckbox->setStyleSheet(UIUtils::getCheckboxStyle(isDark));
        }
        isUpdatingStyles = false;
    }
    
    QWidget::changeEvent(event);
} 