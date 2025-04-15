#ifndef IMAGESLIDER_H
#define IMAGESLIDER_H

#include <QEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QLabel>
#include <vector>

struct SlideInfo {
    QString imagePath;
    QString title;
    QString description;
};

class ImageSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit ImageSlider(QWidget* parent = nullptr);
    void addSlide(const QString& imagePath, const QString& title, const QString& description = QString());
    void startAutoSlide(int interval = 5000);
    void stopAutoSlide();
    void updateTheme(bool isDark);
    void retranslateUI();

    double slideOffset() const { return m_slideOffset; }
    void setSlideOffset(double value) { m_slideOffset = value; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QWidget::event(e);
    }

private slots:
    void nextSlide();
    void previousSlide();
    void goToSlide(int index);
    void updateSlide();
    void updateNavigationButtons() const;

private:
    struct Slide {
        QString imagePath;
        QString title;
        QString description;
    };

    std::vector<Slide> slides;
    int currentIndex;
    QPixmap currentImage;
    QPixmap nextImage;
    QSize originalSize;
    double m_slideOffset;
    QTimer* autoSlideTimer;
    QPropertyAnimation* slideAnimation;
    QPushButton* prevButton{};
    QPushButton* nextButton{};
    bool isDarkTheme;
    bool isSlidingLeft;

    void setupUI();
    void loadImages();
    void applyImageOverlay(QPainter& painter) const;
    void drawSlideText(QPainter& painter) const;
    QString getNavigationButtonStyle() const;
};

#endif // IMAGESLIDER_H 