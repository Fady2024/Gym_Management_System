#ifndef TOPPANEL_H
#define TOPPANEL_H

#include <QWidget>
#include <QColor>
#include <QImage>
#include <QPainterPath>
#include <vector>
#include <functional>

class TopPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit TopPanel(QWidget* parent = nullptr);

    void addActionListener(const std::function<void()>& listener);
    bool isDayValue() const;
    void toggleSwitch();
    void setInitialState(bool isDay);

signals:
    void themeToggled(bool isDark);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    bool isDay;
    float position;
    int width;
    int height;

    QImage sunImage;
    QImage moonImage;

    std::vector<std::function<void()>> actionListeners;

    void animateSwitch();
    static void drawSunbeams(QPainter& painter, int centerX, int centerY);
    std::vector<QPointF> starPositions;
    void drawStars(QPainter& painter) const;
    static QPainterPath createStarPath(float x, float y, int points, float outerRadius, float innerRadius);
    static QColor lerpColor(const QColor& nightColor, const QColor& dayColor, float t);
};

#endif // TOPPANEL_H
