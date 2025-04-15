#include "developerpage.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QDesktopServices>
#include <QUrl>

DeveloperPage::DeveloperPage(QWidget *parent)
    : QWidget(parent)
    , isDarkTheme(false)
    , currentIndex(0)
    , cardWidth(300)
    , cardHeight(450)
    , cardSpacing(120)
    , isFirstShow(true)
    , isDragging(false)
{
    setupUI();
    
    // Install event filter on parent to detect resize events
    if (parent) {
        parent->installEventFilter(this);
    }

    // Setup a timer to periodically update layout
    QTimer* updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &DeveloperPage::updateLayout);
    updateTimer->start(1000); // Update every second
}

void DeveloperPage::setupUI()
{
    setMinimumHeight(600);
    setMinimumWidth(1200);

    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(60, 40, 60, 40);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Create navigation buttons with improved styling
    leftButton = new QPushButton;
    leftButton->setIcon(QIcon(":/Images/chevron-left.png"));
    leftButton->setFixedSize(64, 64);
    leftButton->setCursor(Qt::PointingHandCursor);
    leftButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 92, 246, 0.25), stop:1 rgba(124, 58, 237, 0.25));
            border: none;
            border-radius: 32px;
            padding: 12px;
            position: fixed;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 92, 246, 0.4), stop:1 rgba(124, 58, 237, 0.4));
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 92, 246, 0.5), stop:1 rgba(124, 58, 237, 0.5));
        }
    )");
    
    // Add shadow to left button
    QGraphicsDropShadowEffect* leftShadow = new QGraphicsDropShadowEffect(leftButton);
    leftShadow->setBlurRadius(20);
    leftShadow->setColor(QColor(0, 0, 0, 70));
    leftShadow->setOffset(0, 3);
    leftButton->setGraphicsEffect(leftShadow);

    rightButton = new QPushButton;
    rightButton->setIcon(QIcon(":/Images/chevron-right.png"));
    rightButton->setFixedSize(64, 64);
    rightButton->setCursor(Qt::PointingHandCursor);
    rightButton->setStyleSheet(leftButton->styleSheet());
    
    // Add shadow to right button
    QGraphicsDropShadowEffect* rightShadow = new QGraphicsDropShadowEffect(rightButton);
    rightShadow->setBlurRadius(20);
    rightShadow->setColor(QColor(0, 0, 0, 70));
    rightShadow->setOffset(0, 3);
    rightButton->setGraphicsEffect(rightShadow);
    
    // Create container for cards
    cardsContainer = new QWidget;
    cardsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cardsContainer->setStyleSheet("background: transparent;");
    cardsContainer->installEventFilter(this);

    cardsLayout = new QHBoxLayout(cardsContainer);
    cardsLayout->setSpacing(cardSpacing);
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    cardsLayout->setAlignment(Qt::AlignCenter);

    // Create team member cards
    createTeamMemberCard("Nouran Mahmoud", "Developer", ":/Images/person.png", "Nouran252");
    createTeamMemberCard("Fady Ehab", "Developer", ":/Images/person.png", "FadyEhab6");
    createTeamMemberCard("Fady Gerges", "Developer", ":/Images/person.png", "fady2024");
    createTeamMemberCard("Kareem Amr", "Developer", ":/Images/person.png", "kareem-kio");
    createTeamMemberCard("Peter Emad", "Developer", ":/Images/person.png", "Peter-Emad100");
    createTeamMemberCard("Abdulrahman Ali", "Developer", ":/Images/person.png", "SheikhWalter");
    createTeamMemberCard("Fatma Alzhraa", "Developer", ":/Images/person.png", "fatma0608");

    // Add widgets to main layout with fixed positions for buttons
    mainLayout->addWidget(leftButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(cardsContainer, 1);
    mainLayout->addWidget(rightButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    // Setup auto-scroll timer
    autoScrollTimer = new QTimer(this);
    connect(autoScrollTimer, &QTimer::timeout, this, &DeveloperPage::autoScroll);
    autoScrollTimer->start(5000);

    // Connect button signals
    connect(leftButton, &QPushButton::clicked, this, [this]() {
        stopAutoScroll();
        scrollLeft();
        QTimer::singleShot(3000, this, &DeveloperPage::resumeAutoScroll);
    });

    connect(rightButton, &QPushButton::clicked, this, [this]() {
        stopAutoScroll();
        scrollRight();
        QTimer::singleShot(3000, this, &DeveloperPage::resumeAutoScroll);
    });

    // Initial position update
    updateCardPositions();
}

void DeveloperPage::createTeamMemberCard(const QString& name, const QString& role, const QString& imagePath, const QString& githubUsername)
{
    QWidget* card = new QWidget(this);
    card->setObjectName("teamCard");
    card->setFixedSize(cardWidth, cardHeight);

    // Main vertical layout for the card
    QVBoxLayout* mainCardLayout = new QVBoxLayout(card);
    mainCardLayout->setSpacing(0);
    mainCardLayout->setContentsMargins(20, 20, 20, 20);
    mainCardLayout->setAlignment(Qt::AlignCenter);

    // Container for profile image
    QWidget* imageContainer = new QWidget;
    imageContainer->setObjectName("imageContainer");
    imageContainer->setFixedSize(180, 180);
    QVBoxLayout* imageLayout = new QVBoxLayout(imageContainer);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(0);

    // Profile image
    QLabel* imageLabel = new QLabel;
    imageLabel->setObjectName("profileImage");
    imageLabel->setFixedSize(140, 140);
    imageLabel->setScaledContents(false);
    imageLabel->setAlignment(Qt::AlignCenter);

    QPixmap profileImage(imagePath);
    if (!profileImage.isNull()) {
        QPixmap finalImage(140, 140);
        finalImage.fill(Qt::transparent);
        
        QPainter painter(&finalImage);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Draw glow effect
        QRadialGradient gradient(70, 70, 70);
            gradient.setColorAt(0.5, QColor(139, 92, 246, 180));
            gradient.setColorAt(0.7, QColor(124, 58, 237, 100));
            gradient.setColorAt(1.0, QColor(124, 58, 237, 0));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawEllipse(0, 0, 140, 140);
        
        // Create circular mask for the image
        QPixmap circularImage(120, 120);
        circularImage.fill(Qt::transparent);
        
        QPainter imagePainter(&circularImage);
        imagePainter.setRenderHint(QPainter::Antialiasing);
        imagePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        QPainterPath path;
        path.addEllipse(0, 0, 120, 120);
        imagePainter.setClipPath(path);
        
        QPixmap scaledImage = profileImage.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (120 - scaledImage.width()) / 2;
        int y = (120 - scaledImage.height()) / 2;
        imagePainter.drawPixmap(x, y, scaledImage);
        
        // Draw border
        imagePainter.setPen(QPen(QColor(139, 92, 246), 3));
        imagePainter.drawEllipse(2, 2, 116, 116);
        
        // Draw the circular image centered on the glow
        painter.drawPixmap(10, 10, circularImage);
        
        imageLabel->setPixmap(finalImage);
    }

    imageLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    mainCardLayout->addWidget(imageContainer, 0, Qt::AlignCenter);

    // Spacer between image and text
    mainCardLayout->addSpacing(10);

    // Name label
    QLabel* nameLabel = new QLabel(name);
    nameLabel->setObjectName("nameLabel");
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setFixedHeight(40);
    nameLabel->setStyleSheet(R"(
        color: white;
        font-size: 24px;
        font-weight: 700;
        letter-spacing: 0.5px;
    )");
    mainCardLayout->addWidget(nameLabel, 0, Qt::AlignCenter);

    // Role label
    QLabel* roleLabel = new QLabel(role);
    roleLabel->setObjectName("roleLabel");
    roleLabel->setAlignment(Qt::AlignCenter);
    roleLabel->setFixedHeight(30);
    roleLabel->setStyleSheet(R"(
        color: rgba(209, 213, 219, 0.9);
        font-size: 16px;
        font-weight: 500;
        letter-spacing: 0.3px;
    )");
    mainCardLayout->addWidget(roleLabel, 0, Qt::AlignCenter);

    // Spacer before GitHub button
    mainCardLayout->addSpacing(20);

    // GitHub button container for better positioning
    QWidget* buttonContainer = new QWidget;
    buttonContainer->setFixedSize(48, 48);
    QVBoxLayout* buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);

    // GitHub button
    QPushButton* githubButton = new QPushButton;
    githubButton->setCursor(Qt::PointingHandCursor);
    githubButton->setFixedSize(48, 48);
    githubButton->setIcon(QIcon(":/Images/github.png"));
    githubButton->setIconSize(QSize(24, 24));
    githubButton->setToolTip("View GitHub Profile");
    githubButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(31, 41, 55, 0.7);
            border: 2px solid rgba(139, 92, 246, 0.6);
            border-radius: 24px;
            padding: 6px;
        }
        QPushButton:hover {
            background-color: rgba(31, 41, 55, 0.9);
            border: 2px solid rgba(139, 92, 246, 0.9);
        }
        QPushButton:pressed {
            background-color: rgba(31, 41, 55, 1.0);
        }
    )");

    // Connect GitHub button
    const QString githubUrl = "https://github.com/" + githubUsername;
    connect(githubButton, &QPushButton::clicked, [githubUrl]() {
        QDesktopServices::openUrl(QUrl(githubUrl));
    });

    buttonLayout->addWidget(githubButton, 0, Qt::AlignCenter);
    mainCardLayout->addWidget(buttonContainer, 0, Qt::AlignCenter);
    
    // Add remaining stretch to keep everything properly aligned
    mainCardLayout->addStretch();

    card->setStyleSheet(QString(R"(
        QWidget#teamCard {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(67, 56, 202, 0.98),
                stop:1 rgba(79, 70, 229, 0.98));
            border-radius: 32px;
            border: 3px solid qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(139, 92, 246, 0.9),
                stop:0.5 rgba(124, 58, 237, 0.9),
                stop:1 rgba(99, 102, 241, 0.9));
        }
        QWidget#imageContainer {
            background: transparent;
            border: none;
        }
    )"));

    card->hide();
    teamCards.append(card);
}

void DeveloperPage::SideCardTextAlignment(QWidget* card)
{
    // For side cards, ensure proper text positioning
    QList<QLabel*> labels = card->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        // Adjust label width to fit the scaled card
        label->setFixedWidth(card->width() - 40);
        
        // Force text to be centered horizontally and properly aligned
        label->setAlignment(Qt::AlignHCenter);
        
        // Update styles to ensure text is centered
        if (label->objectName() == "roleLabel") {
            // This is a role label
            label->setStyleSheet(QString(R"(
                color: %1;
                font-size: 15px;
                font-weight: 500;
                letter-spacing: 0.3px;
                line-height: 1.4;
                text-align: center;
                margin-top: 0px;
            )").arg(isDarkTheme ? "rgba(209, 213, 219, 0.9)" : "rgba(75, 85, 99, 0.9)"));
        } else if (label->objectName() == "nameLabel") {
            // This is a name label
            label->setStyleSheet(QString(R"(
                color: %1;
                font-size: 20px;
                font-weight: 700;
                letter-spacing: 0.5px;
                text-align: center;
                margin-top: 0px;
            )").arg(isDarkTheme ? "#FFFFFF" : "#111827"));
        }
    }
}

void DeveloperPage::updateCardPositions()
{
    const int totalCards = teamCards.size();
    if (totalCards == 0) return;

    // Calculate positions
    const int availableWidth = width() - (2 * mainLayout->contentsMargins().left());
    const qreal sideCardScale = 0.85;
    const int sideCardWidth = cardWidth * sideCardScale;
    const int sideCardHeight = cardHeight * sideCardScale;
    const int centerX = (availableWidth - cardWidth) / 2 + mainLayout->contentsMargins().left();
    const int centerY = (height() - cardHeight) / 2;

    // Update button positions
    const int buttonY = centerY + (cardHeight - leftButton->height()) / 2;
    const int buttonSpacing = 50;
    
    leftButton->move(centerX - cardSpacing - sideCardWidth - buttonSpacing - leftButton->width(), buttonY);
    rightButton->move(centerX + cardWidth + cardSpacing + sideCardWidth + buttonSpacing, buttonY);

    // Hide non-visible cards
    for (int i = 0; i < teamCards.size(); i++) {
        if (i != currentIndex && i != (currentIndex + 1) % totalCards && i != ((currentIndex - 1 + totalCards) % totalCards)) {
            teamCards[i]->hide();
        }
        teamCards[i]->setParent(this);
    }

    // Calculate indices
    int prevIndex = ((currentIndex - 1 + totalCards) % totalCards);
    int nextIndex = (currentIndex + 1) % totalCards;

    // Update side cards
    QWidget* prevCard = teamCards[prevIndex];
    QWidget* nextCard = teamCards[nextIndex];
    QWidget* centerCard = teamCards[currentIndex];
    
    // Apply animations
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

    // Previous card animation
    QPropertyAnimation* prevAnim = new QPropertyAnimation(prevCard, "geometry");
    prevAnim->setDuration(300);
    prevAnim->setEndValue(QRect(centerX - cardSpacing - sideCardWidth, 
                               centerY + (cardHeight - sideCardHeight)/2,
                               sideCardWidth, sideCardHeight));
    group->addAnimation(prevAnim);
    
    // Center card animation
    QPropertyAnimation* centerAnim = new QPropertyAnimation(centerCard, "geometry");
    centerAnim->setDuration(300);
    centerAnim->setEndValue(QRect(centerX, centerY, cardWidth, cardHeight));
    group->addAnimation(centerAnim);

    // Next card animation
    QPropertyAnimation* nextAnim = new QPropertyAnimation(nextCard, "geometry");
    nextAnim->setDuration(300);
    nextAnim->setEndValue(QRect(centerX + cardWidth + cardSpacing,
                               centerY + (cardHeight - sideCardHeight)/2,
                               sideCardWidth, sideCardHeight));
    group->addAnimation(nextAnim);

    // Show cards
    prevCard->show();
    centerCard->show();
    nextCard->show();

    // Apply effects
    // Side cards effect
    for (QWidget* sideCard : {prevCard, nextCard}) {
        sideCard->setStyleSheet(QString(R"(
        QWidget#teamCard {
            background: %1;
            border-radius: 32px;
            border: 2px solid %2;
            }
            QWidget#imageContainer {
                background: transparent;
                border: none;
        }
    )").arg(
            isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
        isDarkTheme ? "rgba(255, 255, 255, 0.12)" : "rgba(139, 92, 246, 0.12)"
    ));
    
        QGraphicsDropShadowEffect* sideShadow = new QGraphicsDropShadowEffect(sideCard);
        sideShadow->setBlurRadius(20);
        sideShadow->setColor(isDarkTheme ? QColor(0, 0, 0, 60) : QColor(139, 92, 246, 30));
        sideShadow->setOffset(0, 5);
        sideCard->setGraphicsEffect(sideShadow);
    }

    // Center card effect
    centerCard->setStyleSheet(QString(R"(
            QWidget#teamCard {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 %1,
                stop:1 %2);
                border-radius: 32px;
                border: 3px solid qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                stop:0 rgba(139, 92, 246, 0.9),
                    stop:0.5 rgba(124, 58, 237, 0.9), 
                stop:1 rgba(99, 102, 241, 0.9));
        }
        QWidget#imageContainer {
            background: transparent;
            border: none;
            }
        )").arg(
        isDarkTheme ? "rgba(67, 56, 202, 0.98)" : "rgba(238, 242, 255, 0.98)",
        isDarkTheme ? "rgba(79, 70, 229, 0.98)" : "rgba(224, 231, 255, 0.98)"
        ));
        
    QGraphicsDropShadowEffect* centerShadow = new QGraphicsDropShadowEffect(centerCard);
            centerShadow->setBlurRadius(40);
    centerShadow->setColor(isDarkTheme ? QColor(139, 92, 246, 120) : QColor(139, 92, 246, 80));
    centerShadow->setOffset(0, 0);
    centerCard->setGraphicsEffect(centerShadow);

    // Start animations
    group->start(QAbstractAnimation::DeleteWhenStopped);

    // Update z-order
    centerCard->raise();
    prevCard->lower();
    nextCard->lower();
    
    // Raise navigation buttons
        leftButton->raise();
        rightButton->raise();
}

void DeveloperPage::centerCardsInView()
{
    if (teamCards.isEmpty()) return;
    updateCardPositions();
}

bool DeveloperPage::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == cardsContainer) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mousePressPos = mouseEvent->pos();
            isDragging = true;
            stopAutoScroll();
            return true;
        }
        else if (event->type() == QEvent::MouseMove && isDragging) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            int delta = mousePressPos.x() - mouseEvent->pos().x();
            
            if (qAbs(delta) > 50) { // Threshold for swipe
                if (delta > 0) {
                    scrollRight();
                } else {
                    scrollLeft();
                }
                isDragging = false;
                QTimer::singleShot(3000, this, &DeveloperPage::resumeAutoScroll);
            }
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            isDragging = false;
            QTimer::singleShot(3000, this, &DeveloperPage::resumeAutoScroll);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void DeveloperPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    updateLayout();
    
    // Force card positioning update with animation disabled during resize
    const bool wasAutoScrolling = autoScrollTimer->isActive();
    if (wasAutoScrolling) {
        stopAutoScroll();
    }
    
    updateCardPositions();
    
    if (wasAutoScrolling) {
        resumeAutoScroll();
    }
}

void DeveloperPage::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (isFirstShow) {
        // Initial setup on first show
        updateLayout();
        updateCardPositions();
        isFirstShow = false;
    } else {
        // Ensure everything is positioned correctly when shown again
        QTimer::singleShot(50, this, [this]() {
            updateLayout();
            updateCardPositions();
        });
    }
}

void DeveloperPage::applyCardEffects(QWidget* card, bool isCenter)
{
    // Remove existing effects
    QGraphicsEffect* existingEffect = card->graphicsEffect();
    if (existingEffect) {
        delete existingEffect;
    }

    // Create and apply the opacity effect first
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(card);
    opacityEffect->setOpacity(isCenter ? 1.0 : 0.7);

    // Create shadow effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);

    if (isCenter) {
        shadow->setBlurRadius(40);
        shadow->setColor(isDarkTheme ? QColor(139, 92, 246, 100) : QColor(139, 92, 246, 60));
        shadow->setOffset(0, 0);

        card->setStyleSheet(QString(R"(
            QWidget#teamCard {
                background: %1;
                border-radius: 32px;
                border: 3px solid %2;
                padding: 20px;
            }
        )").arg(
            isDarkTheme ? "rgba(31, 41, 55, 0.98)" : "rgba(255, 255, 255, 0.98)",
            isDarkTheme ? "rgba(139, 92, 246, 0.8)" : "rgba(139, 92, 246, 0.6)"
        ));
    } else {
        shadow->setBlurRadius(25);
        shadow->setColor(isDarkTheme ? QColor(0, 0, 0, 60) : QColor(0, 0, 0, 30));
        shadow->setOffset(0, 6);

        card->setStyleSheet(QString(R"(
            QWidget#teamCard {
                background: %1;
                border-radius: 32px;
                border: 2px solid %2;
                padding: 16px;
            }
        )").arg(
            isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
            isDarkTheme ? "rgba(255, 255, 255, 0.12)" : "rgba(139, 92, 246, 0.12)"
        ));
    }

    // Apply the combined effects
    card->setGraphicsEffect(opacityEffect);
}

void DeveloperPage::scrollLeft()
{
    // Use modulo arithmetic to wrap around
    currentIndex = ((currentIndex - 1) % teamCards.size() + teamCards.size()) % teamCards.size();
    
    // Stop any ongoing auto-scrolling to prevent conflicts
    bool wasAutoScrolling = autoScrollTimer->isActive();
    if (wasAutoScrolling) {
        stopAutoScroll();
    }
    
    // Add a small animation effect when scrolling left
    QWidget* nextCard = teamCards[currentIndex];
    if (nextCard) {
        // Ensure the card is ready to be shown
        nextCard->show();
        nextCard->raise();
    }
    
    // Update card positions with the new index
    updateCardPositions();
    
    // Resume auto-scrolling if it was active before
    if (wasAutoScrolling) {
        // Reset timer to give user time to view the card
        QTimer::singleShot(1000, this, &DeveloperPage::resumeAutoScroll);
    }
}

void DeveloperPage::scrollRight()
{
    // Use modulo arithmetic to wrap around
    currentIndex = (currentIndex + 1) % teamCards.size();
    
    // Stop any ongoing auto-scrolling to prevent conflicts
    bool wasAutoScrolling = autoScrollTimer->isActive();
    if (wasAutoScrolling) {
        stopAutoScroll();
    }
    
    // Add a small animation effect when scrolling right
    QWidget* nextCard = teamCards[currentIndex];
    if (nextCard) {
        // Ensure the card is ready to be shown
        nextCard->show();
        nextCard->raise();
    }
    
    // Update card positions with the new index
    updateCardPositions();
    
    // Resume auto-scrolling if it was active before
    if (wasAutoScrolling) {
        // Reset timer to give user time to view the card
        QTimer::singleShot(1000, this, &DeveloperPage::resumeAutoScroll);
    }
}

void DeveloperPage::autoScroll()
{
    // Always scroll from left to right
    scrollRight();
}

void DeveloperPage::stopAutoScroll()
{
    autoScrollTimer->stop();
}

void DeveloperPage::resumeAutoScroll()
{
    autoScrollTimer->start(5000);
}

void DeveloperPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateCardStyles();
}

void DeveloperPage::updateLayout()
{
    // Update card sizes based on window size
    const QSize windowSize = size();
    
    // Calculate base sizes (maximum sizes for large screens)
    const int baseCardWidth = 300;
    const int baseCardHeight = 450;
    const int baseSpacing = 120;
        
    // Calculate scaling factor based on window width with minimum scale
    qreal widthScaleFactor = qMin(qMax(windowSize.width() / 1400.0, 0.6), 1.0);
    qreal heightScaleFactor = qMin(qMax(windowSize.height() / 800.0, 0.6), 1.0);
    qreal scaleFactor = qMin(widthScaleFactor, heightScaleFactor);
        
    // Update card dimensions with scaling
    cardWidth = static_cast<int>(baseCardWidth * scaleFactor);
    cardHeight = static_cast<int>(baseCardHeight * scaleFactor);
    cardSpacing = static_cast<int>(baseSpacing * scaleFactor);
        
    // Update main layout margins
    int margin = static_cast<int>(60 * scaleFactor);
    mainLayout->setContentsMargins(margin, margin, margin, margin);

    // Force the container to take up the full space
    cardsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Update all cards with new dimensions and styling
    for (QWidget* card : teamCards) {
        card->setFixedSize(cardWidth, cardHeight);
        
        // Update card styling with glass effect
        QString cardStyle = QString(R"(
            QWidget#teamCard {
                background: %1;
                border-radius: %2px;
                border: 2px solid %3;
            }
        )");

        // Create the background gradient string
        QString backgroundGradient = isDarkTheme ? 
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(31, 41, 85, 0.98), stop:1 rgba(31, 41, 55, 0.98))" : 
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0.95), stop:1 rgba(252, 251, 255, 0.95))";

        // Create the border gradient string
        QString borderGradient = isDarkTheme ? 
            "rgba(255, 255, 255, 0.15)" : 
            "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 92, 246, 0.3), stop:1 rgba(124, 58, 237, 0.3))";

        // Apply the styling with proper argument types
        cardStyle = cardStyle.arg(backgroundGradient)
                            .arg(static_cast<int>(32 * scaleFactor))
                            .arg(borderGradient);

        card->setStyleSheet(cardStyle);

        // Update text sizes
        QList<QLabel*> labels = card->findChildren<QLabel*>();
        for (QLabel* label : labels) {
            if (label->objectName() == "nameLabel") {
                label->setStyleSheet(QString(R"(
                    color: %1;
                    font-size: %2px;
                    font-weight: 700;
                    letter-spacing: 0.5px;
                    text-align: center;
                    margin-top: %3px;
                )").arg(
                    isDarkTheme ? "#FFFFFF" : "#1F2937",
                    QString::number(static_cast<int>(24 * scaleFactor)),
                    QString::number(static_cast<int>(8 * scaleFactor))
                ));
                label->setFixedWidth(cardWidth - static_cast<int>(64 * scaleFactor));
            } else if (label->objectName() == "roleLabel") {
                label->setStyleSheet(QString(R"(
                    color: %1;
                    font-size: %2px;
                    font-weight: 500;
                    letter-spacing: 0.3px;
                    line-height: 1.4;
                    text-align: center;
                    margin-top: %3px;
                )").arg(
                    isDarkTheme ? "rgba(209, 213, 219, 0.9)" : "rgba(71, 85, 105, 0.9)",
                    QString::number(static_cast<int>(16 * scaleFactor)),
                    QString::number(static_cast<int>(4 * scaleFactor))
                ));
                label->setFixedWidth(cardWidth - static_cast<int>(64 * scaleFactor));
            }
        }

        // Update GitHub button size and styling
        QList<QPushButton*> buttons = card->findChildren<QPushButton*>();
        for (QPushButton* button : buttons) {
            if (button->toolTip() == "View GitHub Profile") {
                int buttonSize = static_cast<int>(48 * scaleFactor);
                button->setFixedSize(buttonSize, buttonSize);
                button->setIconSize(QSize(static_cast<int>(24 * scaleFactor), 
                                        static_cast<int>(24 * scaleFactor)));
                
                button->setStyleSheet(QString(R"(
                    QPushButton {
                        background-color: %1;
                        border-radius: %2px;
                        border: %3;
                        padding: %4px;
                    }
                    QPushButton:hover {
                        background-color: %5;
                        border: %6;
                    }
                    QPushButton:pressed {
                        background-color: %7;
                    }
                )").arg(
                    isDarkTheme ? "rgba(31, 41, 55, 0.7)" : "rgba(255, 255, 255, 0.9)",
                    QString::number(buttonSize / 2),
                    isDarkTheme ? "2px solid rgba(139, 92, 246, 0.6)" : "2px solid rgba(124, 58, 237, 0.5)",
                    QString::number(static_cast<int>(6 * scaleFactor)),
                    isDarkTheme ? "rgba(31, 41, 55, 0.9)" : "rgba(139, 92, 246, 0.15)",
                    isDarkTheme ? "2px solid rgba(139, 92, 246, 0.9)" : "2px solid rgba(124, 58, 237, 0.7)",
                    isDarkTheme ? "rgba(31, 41, 55, 1.0)" : "rgba(139, 92, 246, 0.25)"
                ));
            }
        }

        // Update shadow effect
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(card);
        shadowEffect->setBlurRadius(30 * scaleFactor);
        shadowEffect->setColor(isDarkTheme ? QColor(0, 0, 0, 100) : QColor(139, 92, 246, 40));
        shadowEffect->setOffset(0, 8 * scaleFactor);
        card->setGraphicsEffect(shadowEffect);
    }

    // Force update of card positions with new dimensions
    QTimer::singleShot(0, this, &DeveloperPage::updateCardPositions);
}

void DeveloperPage::updateCardStyles()
{
    const QString cardStyle = QString(R"(
        QWidget#teamCard {
            background: %1;
            border-radius: 24px;
            border: 1px solid %2;
        }
    )").arg(
        isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
        isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
    );

    for (QWidget* card : teamCards) {
        card->setStyleSheet(cardStyle);

        // Update text colors and GitHub button style
        QList<QLabel*> labels = card->findChildren<QLabel*>();
        for (QLabel* label : labels) {
            if (label->objectName() == "nameLabel") {
                label->setStyleSheet(QString(R"(
                    color: %1;
                    font-size: 24px;
                    font-weight: 700;
                    letter-spacing: 0.5px;
                    text-align: center;
                    margin-top: 10px;
                )").arg(isDarkTheme ? "#FFFFFF" : "#111827"));
            } else if (label->objectName() == "roleLabel") {
                label->setStyleSheet(QString(R"(
                    color: %1;
                    font-size: 16px;
                    font-weight: 500;
                    letter-spacing: 0.3px;
                    line-height: 1.4;
                    text-align: center;
                    margin-top: 5px;
                )").arg(isDarkTheme ? "rgba(209, 213, 219, 0.9)" : "rgba(75, 85, 99, 0.9)"));
            }
        }

        // Update GitHub button style
        QList<QPushButton*> buttons = card->findChildren<QPushButton*>();
        for (QPushButton* button : buttons) {
            if (button->toolTip() == "View GitHub Profile") {
                button->setStyleSheet(QString(R"(
                    QPushButton {
                        background-color: %1;
                        border-radius: 24px;
                        border: 2px solid %2;
                        padding: 6px;
                    }
                    QPushButton:hover {
                        background-color: %3;
                        border: 2px solid %4;
                    }
                    QPushButton:pressed {
                        background-color: %5;
                    }
                )").arg(
                    isDarkTheme ? "rgba(31, 41, 55, 0.7)" : "rgba(255, 255, 255, 0.9)",
                    isDarkTheme ? "2px solid rgba(139, 92, 246, 0.6)" : "2px solid rgba(124, 58, 237, 0.5)",
                    isDarkTheme ? "rgba(31, 41, 55, 0.9)" : "rgba(139, 92, 246, 0.15)",
                    isDarkTheme ? "2px solid rgba(139, 92, 246, 0.9)" : "2px solid rgba(124, 58, 237, 0.7)",
                    isDarkTheme ? "rgba(31, 41, 55, 1.0)" : "rgba(139, 92, 246, 0.25)"
                ));
            }
        }
    }

    updateCardPositions();
}
