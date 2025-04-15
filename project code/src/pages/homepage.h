#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QEvent>

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    void updateTheme(bool isDark) const;
    void updateLayout();
    void retranslateUI();

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* e) override {
        if (e->type() == QEvent::LanguageChange) {
            retranslateUI();
            return true;
        }
        return QWidget::event(e);
    }

private:
    void setupUI();
    QVBoxLayout* mainLayout{};
    QLabel* welcomeLabel{};
    QLabel* subtitleLabel{};
    QLabel* descriptionLabel{};
};

#endif // HOMEPAGE_H 