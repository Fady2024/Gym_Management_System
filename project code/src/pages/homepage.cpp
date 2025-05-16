#include "homepage.h"
#include <QResizeEvent>
#include <qtimer.h>
#include "../../Model/System/timeLogic.h"
#include "Widgets/Notifications/NotificationManager.h"

HomePage::HomePage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void HomePage::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);

    setStyleSheet("background-color: transparent;");

    // Create welcome section with translation
    welcomeLabel = new QLabel;
    welcomeLabel->setStyleSheet("font-size: 32px; font-weight: bold; background-color: transparent;");
    welcomeLabel->setAlignment(Qt::AlignCenter);

    // Create subtitle with translation
    subtitleLabel = new QLabel;
    subtitleLabel->setStyleSheet("font-size: 24px; color: #6B7280; background-color: transparent;");
    subtitleLabel->setAlignment(Qt::AlignCenter);

    // Create description with translation
    descriptionLabel = new QLabel;
    descriptionLabel->setStyleSheet("font-size: 16px; color: #6B7280; background-color: transparent;");
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);


    // Add widgets to layout with proper spacing
    mainLayout->addStretch();
    mainLayout->addWidget(welcomeLabel);
    mainLayout->addSpacing(10);
	mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addStretch();

    // Set minimum size
    setMinimumSize(800, 600);

    // Update texts
    retranslateUI();
}

void HomePage::updateTheme(bool isDark) const
{
    const QString titleColor = isDark ? "#F9FAFB" : "#111827";
    const QString subtitleColor = isDark ? "#9CA3AF" : "#6B7280";
    const QString descriptionColor = isDark ? "#9CA3AF" : "#6B7280";

    welcomeLabel->setStyleSheet(QString("color: %1; font-size: 32px; font-weight: bold; background-color: transparent;")
        .arg(titleColor));
    subtitleLabel->setStyleSheet(QString("color: %1; font-size: 24px; background-color: transparent;")
        .arg(subtitleColor));
    descriptionLabel->setStyleSheet(QString("color: %1; font-size: 16px; background-color: transparent;")
        .arg(descriptionColor));
}

void HomePage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateLayout();
}

void HomePage::updateLayout()
{
    const QSize size = this->size();
    
    // Adjust welcome label font size based on window width
    int titleSize = 32;
    int subtitleSize = 24;
    int descriptionSize = 16;

    if (size.width() < 600) {
        titleSize = 24;
        subtitleSize = 18;
        descriptionSize = 14;
        mainLayout->setContentsMargins(20, 20, 20, 20);
    } else if (size.width() < 800) {
        titleSize = 28;
        subtitleSize = 20;
        descriptionSize = 15;
        mainLayout->setContentsMargins(30, 30, 30, 30);
    } else {
        mainLayout->setContentsMargins(40, 40, 40, 40);
    }
    
    // Update styles while preserving colors
    const QString titleColor = welcomeLabel->palette().color(QPalette::WindowText).name();
    const QString subtitleColor = subtitleLabel->palette().color(QPalette::WindowText).name();
    const QString descriptionColor = descriptionLabel->palette().color(QPalette::WindowText).name();

    welcomeLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold; background-color: transparent;")
        .arg(titleColor)
        .arg(titleSize));

    subtitleLabel->setStyleSheet(QString("color: %1; font-size: %2px; background-color: transparent;")
        .arg(subtitleColor)
        .arg(subtitleSize));

    descriptionLabel->setStyleSheet(QString("color: %1; font-size: %2px; background-color: transparent;")
        .arg(descriptionColor)
        .arg(descriptionSize));
}

void HomePage::retranslateUI()
{
    //welcomeLabel->setText(tr("Welcome to FitFlexPro!"));

    // QTimer* timer = new QTimer(welcomeLabel); // parented to the label for memory management
    // QObject::connect(timer, &QTimer::timeout, [=]() {
    //     welcomeLabel->setText(timeLogicInstance.getFormattedTime() +
    //         "   x" +
    //         QString::number(timeLogicInstance.getMultiplier())
    //         );
    //     });
    // timer->start(100); //updates every one second

    welcomeLabel->setText(tr("Welcome to FitFlexPro"));
    subtitleLabel->setText(tr("Your Personal Fitness Journey Starts Here"));
    descriptionLabel->setText(tr("Track your workouts, monitor your nutrition, and achieve your fitness goals with our comprehensive fitness management system."));
}