#include "leftsidebar.h"
#include <QPainter>
#include <QPainterPath>
#include "Stylesheets/System/leftsidebarStyle.h"

LeftSidebar::LeftSidebar(QWidget *parent)
    : QWidget(parent)
    , isDarkTheme(false)
    , buttonSize(48)
    , buttonSpacing(16)
{
    setupUI();
}

void LeftSidebar::setupUI()
{
    setFixedWidth(80);
    setObjectName("leftSidebar");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(buttonSpacing + 8);
    mainLayout->setContentsMargins(12, 32, 12, 32);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);

    setStyleSheet(R"(
        QWidget#leftSidebar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ede9fe, stop:1 #c7d2fe);
            border-radius: 24px;
        }
    )");

    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 50));
    shadowEffect->setOffset(2, 0);
    setGraphicsEffect(shadowEffect);

    updateButtonStyles();
}

QPushButton* LeftSidebar::createButton(const QString& iconPath, const QString& tooltip, const QString& pageId)
{
    auto button = new QPushButton;
    button->setToolTip(tooltip);
    button->setFixedSize(buttonSize, buttonSize);
    button->setCursor(Qt::PointingHandCursor);
    button->setCheckable(true);
    button->setProperty("pageId", pageId);

    button->setStyleSheet(QString(R"(
        QPushButton {
            border-radius: %1px;
            background: transparent;
        }
    )").arg(buttonSize/3));

    QIcon icon(iconPath);
    button->setIcon(icon);
    button->setIconSize(QSize(buttonSize * 0.7, buttonSize * 0.7));

    auto glowEffect = new QGraphicsDropShadowEffect(button);
    glowEffect->setBlurRadius(18);
    glowEffect->setColor(QColor(139, 92, 246, isDarkTheme ? 80 : 40));
    glowEffect->setOffset(0, 2);
    button->setGraphicsEffect(glowEffect);

    connect(button, &QPushButton::toggled, [button, this](bool checked) {
        if (checked) {
            button->setStyleSheet(QString(R"(
                QPushButton {
                    border: 3px solid #8B5CF6;
                    background: %1;
                    border-radius: %2px;
                }
            )").arg(isDarkTheme ? "rgba(139,92,246,0.12)" : "rgba(139,92,246,0.08)", QString::number(buttonSize/3)));
        } else {
            button->setStyleSheet(QString(R"(
                QPushButton {
                    border-radius: %1px;
                    background: transparent;
                }
                QPushButton:hover {
                    background: rgba(139,92,246,0.05);
                }
            )").arg(buttonSize/3));
        }
    });

    connect(button, &QPushButton::clicked, [this, pageId]() {
        emit pageChanged(pageId);
    });

    return button;
}

void LeftSidebar::addButton(const QString& iconPath, const QString& tooltip, const QString& pageId)
{
    if (buttons.contains(pageId)) return;

    auto button = createButton(iconPath, tooltip, pageId);
    buttons[pageId] = button;
    buttonGroup->addButton(button);
    mainLayout->addWidget(button, 0, Qt::AlignCenter);
}

void LeftSidebar::addSpacer(int height)
{
    mainLayout->addSpacing(height);
}

void LeftSidebar::addStretch(int stretch)
{
    mainLayout->addStretch(stretch);
}

void LeftSidebar::setActiveButton(const QString& pageId)
{
    if (buttons.contains(pageId)) {
        buttons[pageId]->setChecked(true);
        activePageId = pageId;
    }
}

void LeftSidebar::clearButtons()
{
    for (auto button : buttons) {
        mainLayout->removeWidget(button);
        delete button;
    }
    buttons.clear();
}

void LeftSidebar::setFixedButtonSize(int size)
{
    buttonSize = size;
    for (auto button : buttons) {
        button->setFixedSize(buttonSize, buttonSize);
        button->setIconSize(QSize(buttonSize * 0.7, buttonSize * 0.7));
    }
}

void LeftSidebar::setButtonSpacing(int spacing)
{
    buttonSpacing = spacing;
    mainLayout->setSpacing(buttonSpacing + 8);
}

void LeftSidebar::setContentsMargins(int left, int top, int right, int bottom)
{
    mainLayout->setContentsMargins(left, top, right, bottom);
}

void LeftSidebar::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    setStyleSheet(leftsidebarStyle.arg(
        isDark ? "rgba(31, 41, 55, 0.98)" : "rgba(255, 255, 255, 0.98)",
        isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.05)"
    ));

    updateButtonStyles();
}

void LeftSidebar::updateButtonStyles()
{
    // We do not change the button styles here because the highlighting is handled in the toggled slot
} 