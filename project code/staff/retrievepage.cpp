#include "retrievepage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QTableWidget>
#include <QVector>
#include <QHeaderView>
#include <QDebug>

RetrievePage::RetrievePage(UserDataManager* userDataManager, MemberDataManager* memberManager, QWidget* parent)
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

void RetrievePage::setupUI()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create and setup left sidebar
    leftSidebar = new LeftSidebar(this);
    leftSidebar->addButton(":/Images/custom.png", tr("Custom"), "custom");
    connect(leftSidebar, &LeftSidebar::pageChanged, this, &RetrievePage::handlePageChange);

    // Content area
    contentStack = new QStackedWidget;
    contentStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // Custom content
    customContent = new QWidget;
    QVBoxLayout* customLayout = new QVBoxLayout(customContent);
    customLayout->setSpacing(0);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setAlignment(Qt::AlignCenter);

    // Create a container widget for centering
    QWidget* centerContainer = new QWidget;
    centerContainer->setStyleSheet("background: transparent;");
    QVBoxLayout* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(24, 24, 24, 24);
    centerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Card container setup
    QWidget* cardContainer = new QWidget;
    cardContainer->setObjectName("cardContainer");
    cardContainer->setMinimumWidth(480);
    cardContainer->setMaximumWidth(800);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cardContainer->setStyleSheet(R"(
        QWidget#cardContainer {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 24px;
            border: 1px solid rgba(0, 0, 0, 0.1);
        }
    )");

    QVBoxLayout* cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(24);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setAlignment(Qt::AlignHCenter);

    // Add table widget for QVector display
    QTableWidget* tableWidget = new QTableWidget;
    tableWidget->setObjectName("dataTable");
    tableWidget->setMinimumSize(400, 200); // Ensure table is visible
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tableWidget->setStyleSheet(QString(R"(
        QTableWidget {
            background: transparent;
            color: %1;
            font-size: 16px;
            border: none;
            gridline-color: %2;
        }
        QTableWidget::item {
            padding: 8px;
        }
        QHeaderView::section {
            background: %3;
            color: %1;
            padding: 8px;
            border: none;
        }
    )").arg(isDarkTheme ? "#F9FAFB" : "#111827",
    isDarkTheme ? "#4B5563" : "#D1D5DB",
    isDarkTheme ? "rgba(55, 65, 81, 0.95)" : "rgba(229, 231, 235, 0.95)"));

    // Setup table columns
    tableWidget->setColumnCount(2);
    tableWidget->setHorizontalHeaderLabels({ tr("Index"), tr("Value") });
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setShowGrid(true);

    QVector<Member> members = memberManager->getAllMembers();

    if (members.isEmpty()) {
        tableWidget->setRowCount(1);
        tableWidget->setItem(0, 0, new QTableWidgetItem("-"));
        tableWidget->setItem(0, 1, new QTableWidgetItem("No data"));
        tableWidget->setItem(0, 2, new QTableWidgetItem("-"));
    }
    else {
        tableWidget->setColumnCount(4);
        tableWidget->setHorizontalHeaderLabels({ tr("ID"), tr("Name"),tr("Email"), tr("ClassId")});
        tableWidget->setRowCount(members.size());
        for (int i = 0; i < members.size(); ++i) {
            const Member& m = members[i];

            const User &u = userDataManager->getUserDataById(members[i].getUserId());
            QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(m.getId()));
            QTableWidgetItem* nameItem = new QTableWidgetItem(u.getName());
            QTableWidgetItem* emailItem = new QTableWidgetItem(u.getEmail());
            QTableWidgetItem* classIdItem = new QTableWidgetItem(QString::number(m.getClassId()));

            idItem->setTextAlignment(Qt::AlignCenter);
            nameItem->setTextAlignment(Qt::AlignCenter);
            emailItem->setTextAlignment(Qt::AlignCenter);
            classIdItem->setTextAlignment(Qt::AlignCenter);

            tableWidget->setItem(i, 0, idItem);
            tableWidget->setItem(i, 1, nameItem);
            tableWidget->setItem(i, 2, emailItem);
            tableWidget->setItem(i, 3, classIdItem);
        }
    }


    // Debug output to verify population
    //qDebug() << "Table populated with" << data.size() << "rows";

    cardLayout->addWidget(tableWidget);
    centerLayout->addWidget(cardContainer);
    customLayout->addWidget(centerContainer);

    contentStack->addWidget(customContent);

    // Add widgets to main layout
    mainLayout->addWidget(leftSidebar);
    mainLayout->addWidget(contentStack, 1); // Stretch content area

    // Initialize with custom tab
    leftSidebar->setActiveButton("custom");
    handlePageChange("custom");
}

void RetrievePage::showMessageDialog(const QString& message, bool isError)
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

void RetrievePage::handlePageChange(const QString& pageId)
{
    if (pageId == "custom") {
        contentStack->setCurrentWidget(customContent);
    }
}

void RetrievePage::updateTheme(bool isDark)
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

    // Update table widget style if it exists
    if (const auto table = findChild<QTableWidget*>("dataTable")) {
        table->setStyleSheet(QString(R"(
            QTableWidget {
                background: transparent;
                color: %1;
                font-size: 16px;
                border: none;
                gridline-color: %2;
            }
            QTableWidget::item {
                padding: 8px;
            }
            QHeaderView::section {
                background: %3;
                color: %1;
                padding: 8px;
                border: none;
            }
        )").arg(isDark ? "#F9FAFB" : "#111827",
    isDark ? "#4B5563" : "#D1D5DB",
    isDark ? "rgba(55, 65, 81, 0.95)" : "rgba(229, 231, 235, 0.95)"));
    }
}