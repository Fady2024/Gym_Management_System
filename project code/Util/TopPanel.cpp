#include "TopPanel.h"
#include "ThemeManager.h"
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <random>
#include <QDebug>

// Theme toggle panel with animated sun/moon transition
TopPanel::TopPanel(QWidget* parent)
    : QWidget(parent)
    , width(200)
    , height(40)
    , sunImage(":/Images/sun.png")
    , moonImage(":/Images/moon.png")
{
    // Initialize state based on ThemeManager
    isDay = !ThemeManager::getInstance().isDarkTheme();
    position = isDay ? 0.0f : 1.0f;
    
    setFixedSize(width, height);
    
    // Initialize animation timer for smooth transitions
    auto* animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &TopPanel::animateSwitch);
    animationTimer->start(16);

    // Connect to ThemeManager
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
            this, [this](bool isDark) {
                const bool newIsDay = !isDark;
                if (isDay != newIsDay) {
                    isDay = newIsDay;
                    position = isDay ? 0.0f : 1.0f;
                    update();
                }
            });

    // Ensure initial state is correct
    update();
}

// Animate theme switch with smooth position transitions
void TopPanel::animateSwitch()
{
    if (isDay && position > 0.0f) {
        position -= 0.02f;
        update();
    } else if (!isDay && position < 1.0f) {
        position += 0.02f;
        update();
    }
}

// Render the theme toggle panel with animated effects
void TopPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Define track dimensions and position
    constexpr int trackWidth = 80;
    constexpr int trackHeight = 30;
    const int trackX = (width - trackWidth) / 2;
    const int trackY = (height - trackHeight) / 2;

    // Draw animated track background with color interpolation
    const QColor trackColor = lerpColor(QColor(5, 5, 10), QColor(135, 206, 250), position);
    painter.setBrush(trackColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(trackX, trackY, trackWidth, trackHeight, 15, 15);

    // Draw sunbeams effect during day transition
    if (isDay && position < 0.3f) {
        drawSunbeams(painter, trackX + trackWidth / 2, trackY + trackHeight / 2);
    } else if (!isDay && position > 0.3f) {
        drawStars(painter);
    }

    constexpr int thumbRadius = 13;
    const int thumbX = trackX + static_cast<int>((trackWidth - 2 * thumbRadius) * position);
    const int thumbY = trackY + (trackHeight - 2 * thumbRadius) / 2;

    const QImage& thumbImage = (position < 0.5f) ? sunImage : moonImage;
    painter.drawImage(QRect(thumbX, thumbY, 2 * thumbRadius, 2 * thumbRadius), thumbImage);
}

void TopPanel::drawSunbeams(QPainter& painter, int centerX, int centerY)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    constexpr int shortLineLength = 12;
    constexpr int longLineLength = 15;
    constexpr int thickness = 3;
    painter.drawRect(centerX - static_cast<int>(shortLineLength * 2.0f), centerY - (thickness / 2 + 10),
                     shortLineLength * 2, thickness);
    painter.drawRect(centerX - static_cast<int>(longLineLength * 1.0f), centerY - thickness / 2,
                     longLineLength * 2, thickness);
    painter.drawRect(centerX - static_cast<int>(shortLineLength * 2.0f), centerY + 8,
                     shortLineLength * 2, thickness);
}

void TopPanel::drawStars(QPainter& painter) const
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    QVector<QPointF> starPositions = {
        {static_cast<qreal>(width) / 2 - 15, static_cast<qreal>(height) / 2 - 8},
        {static_cast<qreal>(width) / 2 - 15, static_cast<qreal>(height) / 2 + 7},
        {static_cast<qreal>(width) / 2 - 5, static_cast<qreal>(height) / 2 - 1},
        {static_cast<qreal>(width) / 2 - 25, static_cast<qreal>(height) / 2 - 1}
    };

    for (const QPointF& pos : starPositions) {
        QPainterPath starPath = createStarPath(static_cast<float>(pos.x()), static_cast<float>(pos.y()), 5, 3, 1.5);
        painter.drawPath(starPath);
    }
}

QPainterPath TopPanel::createStarPath(float x, float y, int points, float outerRadius, float innerRadius)
{
    QPainterPath path;
    const double angleStep = M_PI / points;

    for (int i = 0; i < 2 * points; ++i) {
        const float r = (i % 2 == 0) ? outerRadius : innerRadius;
        const auto angle = static_cast<float>(i * angleStep - M_PI / 2);
        const float xOffset = x + (r * cos(angle));
        const float yOffset = y + (r * sin(angle));

        if (i == 0)
            path.moveTo(xOffset, yOffset);
        else
            path.lineTo(xOffset, yOffset);
    }
    path.closeSubpath();
    return path;
}

void TopPanel::mousePressEvent(QMouseEvent* event)
{
    constexpr int trackWidth = 80;
    constexpr int trackHeight = 30;
    const int trackX = (width - trackWidth) / 2;
    const int trackY = (height - trackHeight) / 2;
    if (event->position().x() >= trackX && event->position().x() <= trackX + trackWidth &&
        event->position().y() >= trackY && event->position().y() <= trackY + trackHeight) {
        toggleSwitch();
    }
}

void TopPanel::toggleSwitch()
{
    isDay = !isDay;
    ThemeManager::getInstance().setDarkTheme(!isDay);
    update();
}

void TopPanel::addActionListener(const std::function<void()>& listener)
{
    actionListeners.push_back(listener);
}

QColor TopPanel::lerpColor(const QColor& nightColor, const QColor& dayColor, float t)
{
    const float r = (static_cast<float>(dayColor.red()) * (1 - t) + static_cast<float>(nightColor.red()) * t);
    const float g = (static_cast<float>(dayColor.green()) * (1 - t) + static_cast<float>(nightColor.green()) * t);
    const float b = (static_cast<float>(dayColor.blue()) * (1 - t) + static_cast<float>(nightColor.blue()) * t);

    return {static_cast<int>(std::round(r)), static_cast<int>(std::round(g)), static_cast<int>(std::round(b))};
}

bool TopPanel::isDayValue() const
{
    return isDay;
}

void TopPanel::setInitialState(bool isDark)
{
    const bool newIsDay = !isDark;
    if (isDay != newIsDay) {
        isDay = newIsDay;
        position = isDay ? 0.0f : 1.0f;
        update();
    }
}