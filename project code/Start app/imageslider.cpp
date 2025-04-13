#include "imageslider.h"
#include <QPainter>
#include <QPaintEvent>
#include <QHBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "LanguageManager.h"

ImageSlider::ImageSlider(QWidget* parent)
    : QWidget(parent)
    , currentIndex(0)
    , m_slideOffset(0.0)
    , isDarkTheme(false)
    , isSlidingLeft(true)
{
    setupUI();
    
    autoSlideTimer = new QTimer(this);
    connect(autoSlideTimer, &QTimer::timeout, this, &ImageSlider::nextSlide);
    autoSlideTimer->start(5000);
    
    slideAnimation = new QPropertyAnimation(this, "slideOffset");
    slideAnimation->setDuration(500);
    slideAnimation->setStartValue(0.0);
    slideAnimation->setEndValue(1.0);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Connect to LanguageManager
    connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
            this, &ImageSlider::retranslateUI);

    // Initialize slides with empty content - will be populated in retranslateUI
    slides.clear();
    addSlide(":/Images/fitness1.png", "", "");
    addSlide(":/Images/fitness2.png", "", "");
    addSlide(":/Images/fitness3.png", "", "");

    // Update texts
    retranslateUI();
}

void ImageSlider::retranslateUI()
{
    // Clear existing slides
    slides.clear();

    // Add slides with translated text
    addSlide(":/Images/fitness1.png", 
        tr("Transform Your Body"),
        tr("Start your fitness journey today with our personalized workout plans"));
    
    addSlide(":/Images/fitness2.png",
        tr("Find Your Balance"),
        tr("Achieve the perfect balance between strength and flexibility"));
    
    addSlide(":/Images/fitness3.png",
        tr("Join Our Community"),
        tr("Connect with like-minded fitness enthusiasts and reach your goals together"));

    // Force repaint to show new translations
    update();
}

QString ImageSlider::getNavigationButtonStyle() const
{
    constexpr int buttonSize = 40;
    const QString backgroundColor = isDarkTheme ? "rgba(255, 255, 255, 0.15)" : "rgba(0, 0, 0, 0.3)";
    const QString hoverColor = isDarkTheme ? "rgba(255, 255, 255, 0.25)" : "rgba(0, 0, 0, 0.5)";
    const QString pressedColor = isDarkTheme ? "rgba(255, 255, 255, 0.35)" : "rgba(0, 0, 0, 0.6)";
    
    return QString(
        "QPushButton {"
        "   background-color: %1;"
        "   border: none;"
        "   border-radius: %2px;"
        "   padding: 10px;"
        "   color: white;"
        "   min-width: %3px;"
        "   max-width: %3px;"
        "   min-height: %3px;"
        "   max-height: %3px;"
        "}"
        "QPushButton:hover {"
        "   background-color: %4;"
        "   transform: scale(1.05);"
        "}"
        "QPushButton:pressed {"
        "   background-color: %5;"
        "   transform: scale(0.95);"
        "}")
        .arg(backgroundColor)
        .arg(buttonSize/2)
        .arg(buttonSize)
        .arg(hoverColor)
        .arg(pressedColor);
}

void ImageSlider::updateNavigationButtons() const
{
    const QString buttonStyle = getNavigationButtonStyle();
    prevButton->setStyleSheet(buttonStyle);
    nextButton->setStyleSheet(buttonStyle);
}

void ImageSlider::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    updateNavigationButtons();
}

void ImageSlider::setupUI()
{
    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    prevButton = new QPushButton(this);
    nextButton = new QPushButton(this);
    
    prevButton->setIcon(QIcon(":/Images/chevron-left.png"));
    nextButton->setIcon(QIcon(":/Images/chevron-right.png"));

    const int buttonSize = 48;
    prevButton->setFixedSize(buttonSize, buttonSize);
    nextButton->setFixedSize(buttonSize, buttonSize);
    
    updateNavigationButtons();

    const int iconSize = buttonSize * 2/3;
    prevButton->setIconSize(QSize(iconSize, iconSize));
    nextButton->setIconSize(QSize(iconSize, iconSize));
    
    connect(prevButton, &QPushButton::clicked, this, &ImageSlider::previousSlide);
    connect(nextButton, &QPushButton::clicked, this, &ImageSlider::nextSlide);
}

void ImageSlider::addSlide(const QString& imagePath, const QString& title, const QString& description)
{
    slides.push_back({imagePath, title, description});
    
    if (slides.size() == 1) {
        currentImage = QPixmap(imagePath);
        if (!currentImage.isNull()) {
            originalSize = currentImage.size();
        }
        update();
    }
}

void ImageSlider::goToSlide(int index)
{
    if (index == currentIndex || index < 0 || index >= static_cast<int>(slides.size()))
        return;
    
    nextImage = QPixmap(slides[index].imagePath);
    m_slideOffset = 0.0;
    slideAnimation->start();
    
    connect(slideAnimation, &QPropertyAnimation::finished, this, [this, index]() {
        currentIndex = index;
        currentImage = nextImage;
        m_slideOffset = 0.0;
        update();
    }, Qt::SingleShotConnection);
}

void ImageSlider::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    const int buttonSize = prevButton->width();
    const int verticalCenter = height() / 2 - buttonSize / 2;
    
    const int horizontalMargin = qMax(16, width() / 40);
    
    prevButton->move(horizontalMargin, verticalCenter);
    nextButton->move(width() - nextButton->width() - horizontalMargin, verticalCenter);
    
    update();
}

void ImageSlider::updateSlide()
{
    update();
}

void ImageSlider::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!currentImage.isNull()) {
        painter.setClipRect(rect());
        
        QSize scaledSize = currentImage.size();
        scaledSize.scale(size(), Qt::KeepAspectRatioByExpanding);
        
        int x = (width() - scaledSize.width()) / 2;
        int y = (height() - scaledSize.height()) / 2;
        
        if (x > 0) x = 0;
        if (y > 0) y = 0;
        
        int offset = static_cast<int>(width() * m_slideOffset);
        if (!isSlidingLeft) {
            offset = -offset;
        }
        
        painter.drawPixmap(x + offset, y, 
            currentImage.scaled(scaledSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        
        applyImageOverlay(painter);
        drawSlideText(painter);
    }
    
    if (!nextImage.isNull() && m_slideOffset > 0.0) {
        QSize nextScaledSize = nextImage.size();
        nextScaledSize.scale(size(), Qt::KeepAspectRatioByExpanding);
        
        int x = (width() - nextScaledSize.width()) / 2;
        int y = (height() - nextScaledSize.height()) / 2;
        
        if (x > 0) x = 0;
        if (y > 0) y = 0;
        
        int offset = static_cast<int>(width() * (1.0 - m_slideOffset));
        if (isSlidingLeft) {
            offset = -offset;
        }
        
        painter.drawPixmap(x + offset, y, 
            nextImage.scaled(nextScaledSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        
        applyImageOverlay(painter);
        drawSlideText(painter);
    }
}

void ImageSlider::applyImageOverlay(QPainter& painter) const
{
    QLinearGradient gradient(0, 0, width(), 0);
    gradient.setColorAt(0, QColor(0, 0, 0, 180));
    gradient.setColorAt(0.3, QColor(0, 0, 0, 140));
    gradient.setColorAt(0.7, QColor(0, 0, 0, 100));
    gradient.setColorAt(1, QColor(0, 0, 0, 60));
    
    painter.fillRect(rect(), gradient);
}

void ImageSlider::drawSlideText(QPainter& painter) const
{
    if (currentIndex >= 0 && currentIndex < static_cast<int>(slides.size())) {
        const auto& slide = slides[currentIndex];
        
        const int baseFontSize = qMax(20, qMin(42, width() / 20));
        QFont titleFont = font();
        titleFont.setPointSize(baseFontSize);
        titleFont.setWeight(QFont::DemiBold);
        painter.setFont(titleFont);
        painter.setPen(Qt::white);
        
        const int margin = qMax(40, width() / 16);
        QRect textRect = rect().adjusted(margin, 0, -margin, 0);
        
        const int bottomMargin = qMax(80, height() / 6);
        textRect.setBottom(textRect.bottom() - bottomMargin);
        
        const int maxTextWidth = qMin(width() - margin * 2, 800);
        textRect.setWidth(maxTextWidth);

        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignBottom | Qt::TextWordWrap, slide.title);

        const int underlineWidth = qMax(40, qMin(64, width() / 16));
        const int underlineHeight = qMax(3, width() / 200);
        const QRect underlineRect(textRect.left(), textRect.bottom() + 16, underlineWidth, underlineHeight);
        painter.fillRect(underlineRect, QColor(0x8B5CF6));
    }
}

void ImageSlider::nextSlide()
{
    if (slides.empty()) return;
    const int nextIndex = static_cast<int>((currentIndex + 1) % static_cast<int>(slides.size()));
    isSlidingLeft = true;
    goToSlide(nextIndex);
}

void ImageSlider::previousSlide()
{
    if (slides.empty()) return;
    const int prevIndex = static_cast<int>((currentIndex - 1 + static_cast<int>(slides.size())) % static_cast<int>(slides.size()));
    isSlidingLeft = false;
    goToSlide(prevIndex);
} 
