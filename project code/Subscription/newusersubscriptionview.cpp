#include "subscriptionstatuspage.h"
#include <QDebug>
#include <QPixmap>
#include <QFrame>
#include <QScrollArea>
#include <QSizePolicy>
#include <QRegularExpression>

NewUserSubscriptionView::NewUserSubscriptionView(QWidget *parent)
    : QWidget(parent)
    , isDarkTheme(false)
{
    setupUI();
}

void NewUserSubscriptionView::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setAlignment(Qt::AlignCenter);

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

    // Create status card with elevated design
    statusCard = new QWidget;
    statusCard->setObjectName("statusCard");
    statusCard->setMinimumWidth(580);
    statusCard->setMaximumWidth(800);
    statusCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* cardLayout = new QVBoxLayout(statusCard);
    cardLayout->setSpacing(28);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    
    // Add emoji to heading for visual appeal
    emojiLabel = new QLabel("🏋️‍♂️");
    emojiLabel->setObjectName("emojiLabel");
    emojiLabel->setAlignment(Qt::AlignCenter);
    emojiLabel->setStyleSheet("QLabel#emojiLabel { font-size: 48px; margin-bottom: 12px; }");
    cardLayout->addWidget(emojiLabel, 0, Qt::AlignHCenter);
    
    // Create heading
    headingLabel = new QLabel();
    headingLabel->setObjectName("promoHeading");
    headingLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(headingLabel);
    
    // Add subtitle
    subtitleLabel = new QLabel();
    subtitleLabel->setObjectName("promoSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(subtitleLabel);
    
    // Add divider
    divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setObjectName("promoDivider");
    cardLayout->addWidget(divider);
    
    // Add benefits container
    benefitsContainer = new QWidget();
    benefitsContainer->setObjectName("benefitsContainer");
    
    QVBoxLayout* benefitsLayout = new QVBoxLayout(benefitsContainer);
    benefitsLayout->setSpacing(16);
    benefitsLayout->setContentsMargins(20, 20, 20, 20);
    
    // Create benefit labels with proper object names
    for (int i = 0; i < 5; ++i) {
        QLabel* benefitLabel = new QLabel();
        benefitLabel->setWordWrap(true);
        benefitLabel->setObjectName(QString("benefitLabel%1").arg(i));
        benefitsLayout->addWidget(benefitLabel);
    }
    
    cardLayout->addWidget(benefitsContainer);
    
    // Add pricing container
    pricingContainer = new QWidget();
    pricingContainer->setObjectName("pricingContainer");
    QHBoxLayout* pricingLayout = new QHBoxLayout(pricingContainer);
    
    QLabel* currencyLabel = new QLabel();
    currencyLabel->setObjectName("currencyLabel");
    
    QLabel* priceLabel = new QLabel("29.99");
    priceLabel->setObjectName("priceValue");
    
    QLabel* periodLabel = new QLabel();
    periodLabel->setObjectName("periodLabel");
    
    pricingLayout->addStretch();
    pricingLayout->addWidget(currencyLabel);
    pricingLayout->addWidget(priceLabel);
    pricingLayout->addWidget(periodLabel);
    pricingLayout->addStretch();
    
    cardLayout->addWidget(pricingContainer);
    
    // Add subscribe button
    subscribeButton = new QPushButton();
    subscribeButton->setObjectName("subscribePromoButton");
    subscribeButton->setCursor(Qt::PointingHandCursor);
    subscribeButton->setMinimumWidth(300);
    subscribeButton->setMinimumHeight(60);
    connect(subscribeButton, &QPushButton::clicked, this, &NewUserSubscriptionView::subscribeRequested);
    cardLayout->addWidget(subscribeButton, 0, Qt::AlignHCenter);
    
    // Add note label
    noteLabel = new QLabel();
    noteLabel->setObjectName("noteLabel");
    noteLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(noteLabel);
    
    containerLayout->addWidget(statusCard, 0, Qt::AlignHCenter);
    containerLayout->addStretch(1);
    
    scrollArea->setWidget(contentContainer);
    mainLayout->addWidget(scrollArea);

    // Initialize translations at the end
    retranslateUI();
    setupCardStyles();
}

void NewUserSubscriptionView::retranslateUI()
{
    headingLabel->setText(tr("Start Your Fitness Journey"));
    subtitleLabel->setText(tr("Transform your workouts with FitFlex Pro"));
    
    // Update benefits
    QList<QLabel*> benefitLabels = benefitsContainer->findChildren<QLabel*>();
    QStringList benefits = {
        tr("✨ Premium fitness classes and personal training"),
        tr("📊 Personalized workout plans"),
        tr("🥗 Nutrition guidance and meal planning"),
        tr("📈 Progress tracking and analytics"),
        tr("🔒 Access to exclusive fitness content")
    };
    
    for (int i = 0; i < benefitLabels.size() && i < benefits.size(); ++i) {
        QLabel* label = benefitLabels[i];
        if (label) {
            label->setText(benefits[i]);
        }
    }
    
    // Update pricing labels
    if (QLabel* currencyLabel = pricingContainer->findChild<QLabel*>("currencyLabel")) {
        currencyLabel->setText(tr("$"));
    }
    if (QLabel* periodLabel = pricingContainer->findChild<QLabel*>("periodLabel")) {
        periodLabel->setText(tr("/month"));
    }
    
    // Update button and note
    subscribeButton->setText(tr("Start Your Journey"));
    noteLabel->setText(tr("Join thousands of members improving their fitness"));
}

void NewUserSubscriptionView::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    setupCardStyles();
    retranslateUI();
}

void NewUserSubscriptionView::setupCardStyles()
{
    // Update heading and subtitle colors
    headingLabel->setStyleSheet(QString("QLabel#promoHeading { color: %1; font-size: 32px; font-weight: 700; letter-spacing: -0.5px; }")
        .arg(isDarkTheme ? "#F7FAFC" : "#2D3748"));
    
    subtitleLabel->setStyleSheet(QString("QLabel#promoSubtitle { color: %1; font-size: 18px; margin-bottom: 12px; }")
        .arg(isDarkTheme ? "#A0AEC0" : "#718096"));
    
    // Update divider
    divider->setStyleSheet(QString("QFrame#promoDivider { background-color: %1; max-height: 1px; }")
        .arg(isDarkTheme ? "rgba(160, 174, 192, 0.2)" : "rgba(113, 128, 150, 0.2)"));
    
    // Update benefits container
    benefitsContainer->setStyleSheet(QString("QWidget#benefitsContainer { background-color: %1; border-radius: 12px; padding: 8px; }")
        .arg(isDarkTheme ? "rgba(45, 55, 72, 0.5)" : "rgba(237, 242, 247, 0.7)"));
    
    // Update benefit labels
    QList<QLabel*> benefitLabels = benefitsContainer->findChildren<QLabel*>();
    for (QLabel* label : benefitLabels) {
        label->setStyleSheet(QString("font-size: 17px; color: %1; line-height: 1.6;")
            .arg(isDarkTheme ? "#E2E8F0" : "#4A5568"));
    }
    
    // Update pricing container
    QLabel* currencyLabel = pricingContainer->findChild<QLabel*>("currencyLabel");
    if (currencyLabel) {
        currencyLabel->setStyleSheet(QString("QLabel#currencyLabel { color: %1; font-size: 24px; font-weight: 600; }")
            .arg(isDarkTheme ? "#60A5FA" : "#3B82F6"));
    }
    
    QLabel* priceLabel = pricingContainer->findChild<QLabel*>("priceValue");
    if (priceLabel) {
        priceLabel->setStyleSheet(QString("QLabel#priceValue { color: %1; font-size: 36px; font-weight: 700; }")
            .arg(isDarkTheme ? "#60A5FA" : "#3B82F6"));
    }
    
    QLabel* periodLabel = pricingContainer->findChild<QLabel*>("periodLabel");
    if (periodLabel) {
        periodLabel->setStyleSheet(QString("QLabel#periodLabel { color: %1; font-size: 18px; }")
            .arg(isDarkTheme ? "#A0AEC0" : "#718096"));
    }
    
    // Update button with different gradients for dark/light mode
    QString buttonGradient = isDarkTheme ?
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #9F7AEA, stop:1 #805AD5)" : 
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #6D28D9)";
    
    QString buttonHoverGradient = isDarkTheme ?
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8B5CF6, stop:1 #7C3AED)" : 
        "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #7C3AED, stop:1 #5B21B6)";
    
    subscribeButton->setStyleSheet(QString(R"(
        QPushButton#subscribePromoButton {
            background: %1;
            color: white;
            border: none;
            border-radius: 16px;
            font-size: 20px;
            font-weight: 700;
            padding: 16px 40px;
        }
        QPushButton#subscribePromoButton:hover {
            background: %2;
        }
    )").arg(buttonGradient, buttonHoverGradient));
    
    // Update note label
    noteLabel->setStyleSheet(QString("QLabel#noteLabel { color: %1; font-size: 14px; margin-top: 16px; }")
        .arg(isDarkTheme ? "#A0AEC0" : "#718096"));
    
    // Update card style
    QString gradientBorder = isDarkTheme ? 
        "qradialgradient(cx:0.5, cy:0.5, radius:1, fx:0.5, fy:0.5, stop:0 #9F7AEA, stop:1 #805AD5)" : 
        "qradialgradient(cx:0.5, cy:0.5, radius:1, fx:0.5, fy:0.5, stop:0 #8B5CF6, stop:1 #6D28D9)";
    
    statusCard->setStyleSheet(QString(R"(
        QWidget#statusCard {
            background: %1;
            border-radius: 24px;
            border: 2px solid %2;
        }
    )").arg(
        isDarkTheme ? 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(30, 41, 59, 0.98), stop:1 rgba(15, 23, 42, 0.98))" : 
        "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0.98), stop:1 rgba(248, 250, 252, 0.98))",
        gradientBorder
    ));
    
    // Update shadow
    QGraphicsDropShadowEffect* cardShadow = qobject_cast<QGraphicsDropShadowEffect*>(statusCard->graphicsEffect());
    if (cardShadow) {
        cardShadow->setColor(QColor(0, 0, 0, isDarkTheme ? 80 : 40));
    }
}

void NewUserSubscriptionView::updateLayout()
{
    // Ensure card has appropriate width
    int cardWidth = qMin(800, qMax(560, width() - 80));
    statusCard->setFixedWidth(cardWidth);
} 