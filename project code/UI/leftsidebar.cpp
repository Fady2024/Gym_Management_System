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
    setFixedWidth(72);
    setObjectName("leftSidebar");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(buttonSpacing);
    mainLayout->setContentsMargins(12, 24, 12, 24);
    mainLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);

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

    QIcon normalIcon;
    QIcon activeIcon;
    QPixmap originalPixmap(iconPath);
    if (!originalPixmap.isNull()) {
        QPixmap normalPixmap = originalPixmap;
        QPainter normalPainter(&normalPixmap);
        normalPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        normalPainter.fillRect(normalPixmap.rect(), isDarkTheme ? QColor(156, 163, 175) : QColor(75, 85, 99));
        normalIcon.addPixmap(normalPixmap);

        QPixmap activePixmap = originalPixmap;
        QPainter activePainter(&activePixmap);
        activePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        activePainter.fillRect(activePixmap.rect(), QColor(255, 255, 255));
        activeIcon.addPixmap(activePixmap);
    }

    button->setIcon(normalIcon);
    button->setIconSize(QSize(buttonSize/2, buttonSize/2));

    auto glowEffect = new QGraphicsDropShadowEffect(button);
    glowEffect->setBlurRadius(15);
    glowEffect->setColor(QColor(139, 92, 246, isDarkTheme ? 80 : 40));
    glowEffect->setOffset(0, 0);
    button->setGraphicsEffect(glowEffect);

    connect(button, &QPushButton::toggled, [button, normalIcon, activeIcon](bool checked) {
        button->setIcon(checked ? activeIcon : normalIcon);
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
        button->setIconSize(QSize(buttonSize/2, buttonSize/2));
    }
}

void LeftSidebar::setButtonSpacing(int spacing)
{
    buttonSpacing = spacing;
    mainLayout->setSpacing(buttonSpacing);
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
    const QString buttonStyle = QString(R"(
        QPushButton {
            background: %1;
            border: none;
            border-radius: %5px;
            padding: 0;
            margin: 0;
            qproperty-iconSize: %6px %6px;
        }
        QPushButton:hover:!checked {
            background: %2;
        }
        QPushButton:checked {
            background: %3;
        }
        QPushButton:checked:hover {
            background: %4;
        }
    )").arg(
        isDarkTheme ? "rgba(31, 41, 55, 0.5)" : "rgba(255, 255, 255, 0.5)",
        isDarkTheme ? "rgba(139, 92, 246, 0.15)" : "rgba(139, 92, 246, 0.1)",
        isDarkTheme ? "#8B5CF6" : "#7C3AED",
        isDarkTheme ? "#7C3AED" : "#6D28D9",
        QString::number(buttonSize / 3),
        QString::number(buttonSize / 2)
    );

    for (auto button : buttons) {
        button->setStyleSheet(buttonStyle);
    }
} 