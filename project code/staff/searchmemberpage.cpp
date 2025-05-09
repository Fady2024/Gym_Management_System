#include "searchmemberpage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QLineEdit>

SearchMember::SearchMember(UserDataManager* userDataManager, MemberDataManager* memberManager, QWidget* parent)
    : QWidget(parent)
    , userDataManager(userDataManager)
    , memberManager(memberManager)
    , mainLayout(nullptr)
    , leftSidebar(nullptr)
    , contentStack(nullptr)
    , customContent(nullptr)
    , isDarkTheme(false)
    , currentUserId(0)
{
    setupUI();
}

void SearchMember::setupUI()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Left sidebar
    leftSidebar = new LeftSidebar(this);
    leftSidebar->addButton(":/Images/custom.png", tr("Search"), "custom");
    connect(leftSidebar, &LeftSidebar::pageChanged, this, &SearchMember::handlePageChange);

    // Content area
    contentStack = new QStackedWidget;
    contentStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // Search content
    customContent = new QWidget;
    QVBoxLayout* customLayout = new QVBoxLayout(customContent);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setAlignment(Qt::AlignCenter);

    QWidget* centerContainer = new QWidget;
    centerContainer->setStyleSheet("background: transparent;");
    QVBoxLayout* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(24, 24, 24, 24);
    centerLayout->setAlignment(Qt::AlignCenter);

    QWidget* cardContainer = new QWidget;
    cardContainer->setObjectName("cardContainer");
    cardContainer->setMinimumWidth(480);
    cardContainer->setMaximumWidth(800);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cardContainer->setStyleSheet(QString(R"(
        QWidget#cardContainer {
            background: %1;
            border-radius: 24px;
            border: 1px solid %2;
        }
    )").arg(
    isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
    isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
));

    QVBoxLayout* cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(16);
    cardLayout->setContentsMargins(32, 32, 32, 32);

    QLabel* titleLabel = new QLabel(tr("Search Member"));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("color: %1; font-size: 20px; font-weight: 600;")
        .arg(isDarkTheme ? "#F9FAFB" : "#111827"));
    cardLayout->addWidget(titleLabel);

    QLineEdit* searchInput = new QLineEdit;
    searchInput->setPlaceholderText("Enter Member ID");
    searchInput->setStyleSheet(QString(
        "padding: 10px; font-size: 16px; color: %1; background: %2;"
        "border: 1px solid %3; border-radius: 6px;")
        .arg(isDarkTheme ? "#F9FAFB" : "#111827")
        .arg(isDarkTheme ? "#1F2937" : "#FFFFFF")
        .arg(isDarkTheme ? "#4B5563" : "#D1D5DB"));
    cardLayout->addWidget(searchInput);

    QPushButton* searchButton = new QPushButton("Search");
    searchButton->setStyleSheet(QString(
        "padding: 10px 16px; font-size: 16px; background-color: %1;"
        "color: #FFFFFF; border-radius: 6px;")
        .arg(isDarkTheme ? "#2563EB" : "#3B82F6"));
    cardLayout->addWidget(searchButton, 0, Qt::AlignCenter);

    QLabel* searchResult = new QLabel;
    searchResult->setAlignment(Qt::AlignCenter);
    searchResult->setStyleSheet(QString("font-size: 16px; font-weight: 500; color: %1;")
        .arg(isDarkTheme ? "#E5E7EB" : "#1F2937"));
    cardLayout->addWidget(searchResult);

    centerLayout->addWidget(cardContainer, 0, Qt::AlignCenter);
    customLayout->addWidget(centerContainer);
    contentStack->addWidget(customContent);

    mainLayout->addWidget(leftSidebar);
    mainLayout->addWidget(contentStack);

    leftSidebar->setActiveButton("custom");
    handlePageChange("custom");

    connect(searchButton, &QPushButton::clicked, this, [=]() {
        handleSearchClicked(searchInput, searchResult);
        });
}



void SearchMember::showMessageDialog(const QString& message, bool isError)
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle(isError ? tr("Error") : tr("Success"));
    dialog->setFixedSize(400, 240);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_TranslucentBackground);

    QWidget* container = new QWidget(dialog);
    container->setObjectName("messageContainer");
    container->setStyleSheet(QString(R"(
        QWidget#messageContainer {
            background: %1;
            border-radius: 16px;
            border: 1px solid %2;
        }
    )").arg(
    isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
    isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
));

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setSpacing(24);
    layout->setContentsMargins(32, 32, 32, 32);

    QLabel* iconLabel = new QLabel;
    iconLabel->setFixedSize(64, 64);
    iconLabel->setScaledContents(false);
    iconLabel->setAlignment(Qt::AlignCenter);

    QPixmap iconPixmap(isError ? ":/Images/error.png" : ":/Images/success.png");
    if (!iconPixmap.isNull()) {
        iconPixmap = iconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        iconLabel->setPixmap(iconPixmap);
    }

    QLabel* messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 500;")
        .arg(isDarkTheme ? "#F9FAFB" : "#111827"));

    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(messageLabel);

    QVBoxLayout* dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(container);

    QPoint parentCenter = this->mapToGlobal(this->rect().center());
    dialog->move(parentCenter.x() - dialog->width() / 2,
        parentCenter.y() - dialog->height() / 2);

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(dialog);
    container->setGraphicsEffect(opacityEffect);

    QPropertyAnimation* fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    QTimer* dismissTimer = new QTimer(dialog);
    dismissTimer->setSingleShot(true);
    connect(dismissTimer, &QTimer::timeout, [dialog, fadeOut]() {
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        });
    connect(fadeOut, &QPropertyAnimation::finished, dialog, &QDialog::accept);

    dialog->show();
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    dismissTimer->start(2000);

    dialog->exec();
    dialog->deleteLater();
}

void SearchMember::handlePageChange(const QString& pageId)
{
    if (pageId == "custom") {
        contentStack->setCurrentWidget(customContent);
    }
}

void SearchMember::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    leftSidebar->updateTheme(isDark);

    if (const auto container = findChild<QWidget*>("cardContainer")) {
        container->setStyleSheet(QString(R"(
            QWidget#cardContainer {
                background: %1;
                border-radius: 24px;
                border: 1px solid %2;
            }
        )").arg(
    isDark ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
    isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"
));
    }
}
void SearchMember::handleSearchClicked(QLineEdit* input, QLabel* resultLabel)
{
    bool ok;
    int memberId = input->text().toInt(&ok);
    if (!ok) {
        resultLabel->setText("Invalid ID format.");
        return;
    }

    Member member = memberManager->getMemberById(memberId);
    //if (member) {
        resultLabel->setText(QString("Member Found: %1").arg(member.getClassId()));
    //}
    //else {
        //resultLabel->setText("No member found with this ID.");
    //}
}
