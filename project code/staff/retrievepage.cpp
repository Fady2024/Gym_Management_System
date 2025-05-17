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
#include <QLineEdit>
#include <QFormLayout>
#include <QTableWidget>
#include <QMessageBox>
#include <QPushButton>
RetrievePage::RetrievePage(UserDataManager *userDataManager, MemberDataManager *memberManager, QWidget *parent)
    : QWidget(parent), userDataManager(userDataManager), memberManager(memberManager), mainLayout(nullptr), leftSidebar(nullptr), contentStack(nullptr), customContent(nullptr), isDarkTheme(false), currentUserId(0), searchEdit(nullptr), tableWidget(nullptr)
{
    setupUI();
}

void RetrievePage::setupUI()
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    leftSidebar = new LeftSidebar(this);
    leftSidebar->addButton(":/Images/member-search.png", tr("Retrieve Members"), "custom");
    leftSidebar->addButton(":/Images/add-member.png", tr("Add Members"), "add-member");
    connect(leftSidebar, &LeftSidebar::pageChanged, this, &RetrievePage::handlePageChange);

    contentStack = new QStackedWidget;
    contentStack->setStyleSheet("QStackedWidget { background: transparent; }");

    // Initialize addMemberPage early
    addMemberPage = new AddMemberPage(userDataManager, memberManager, this);
    contentStack->addWidget(addMemberPage);

    // Rest of the custom content setup
    customContent = new QWidget;
    QVBoxLayout *customLayout = new QVBoxLayout(customContent);
    customLayout->setSpacing(0);
    customLayout->setContentsMargins(0, 0, 0, 0);
    customLayout->setAlignment(Qt::AlignCenter);

    QWidget *centerContainer = new QWidget;
    centerContainer->setStyleSheet("background: transparent;");
    QVBoxLayout *centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setSpacing(0);
    centerLayout->setContentsMargins(24, 24, 24, 24);
    centerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QWidget *cardContainer = new QWidget;
    cardContainer->setObjectName("cardContainer");
    cardContainer->setMinimumWidth(480);
    cardContainer->setMaximumWidth(800);
    cardContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cardContainer->setStyleSheet(QString(R"(QWidget#cardContainer{background-color: %1;border-radius: 20px;padding: 16px;border: 1px solid %2;})").arg(isDarkTheme ? "rgba(139, 92, 246, 0.1)" : "rgba(139, 92, 246, 0.05)", /* Background */ isDarkTheme ? "rgba(139, 92, 246, 0.3)" : "#BFAEF5"));

    QVBoxLayout *cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setSpacing(24);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setAlignment(Qt::AlignHCenter);

    // Search bar
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("Search by name or email...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setStyleSheet(QString(R"(QLineEdit {padding: 8px;font-size: 16px;color: %1;background: %2;border: 1px solid %3;border-radius: 6px;}QLineEdit:focus {border: 1px solid %4;background: %5;}QLineEdit::placeholder {color: %6;})").arg(isDarkTheme ? "#F9FAFB" : "#111827", /* Text color */ isDarkTheme ? "rgba(31, 41, 55, 0.5)" : "#F9FAFB", /* Background */ isDarkTheme ? "#4B5563" : "#D1D5DB", /* Border */ isDarkTheme ? "#8B5CF6" : "#6D28D9", /* Focus border */ isDarkTheme ? "rgba(31, 41, 55, 0.8)" : "white", /* Focus background */ isDarkTheme ? "#9CA3AF" : "#6B7280" /* Placeholder color */));
    cardLayout->addWidget(searchEdit);

    // Table setup
    tableWidget = new QTableWidget;
    tableWidget->setObjectName("dataTable");
    tableWidget->setMinimumSize(400, 200);
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tableWidget->setColumnCount(3);
    tableWidget->setHorizontalHeaderLabels({tr("ID"), tr("Name"), tr("Email")});
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->setShowGrid(true);
    tableWidget->setStyleSheet(QString(R"(        QTableWidget {            background: %4;            color: %1;            font-size: 16px;            border: none;            gridline-color: %2;        }        QTableWidget::item {            padding: 8px;            color: %1;            background: %4;        }        QTableWidget::item:selected {            background: %5;            color: %1;        }        QHeaderView::section {            background: %3;            color: %1;            padding: 8px;            border: none;        }    )").arg(isDarkTheme ? "#F9FAFB" : "#111827", /* Text color */ isDarkTheme ? "#4B5563" : "#D1D5DB", /* Grid color */ isDarkTheme ? "rgba(55, 65, 81, 0.95)" : "rgba(229, 231, 235, 0.95)", /* Header background */ isDarkTheme ? "rgba(31, 41, 55, 0.5)" : "rgba(255, 255, 255, 0.8)", /* Cell background */ isDarkTheme ? "rgba(139, 92, 246, 0.2)" : "rgba(139, 92, 246, 0.1)" /* Selection background */));

    cardLayout->addWidget(tableWidget);
    centerLayout->addWidget(cardContainer);
    customLayout->addWidget(centerContainer);
    contentStack->addWidget(customContent);

    mainLayout->addWidget(leftSidebar);
    mainLayout->addWidget(contentStack, 1);

    leftSidebar->setActiveButton("custom");
    handlePageChange("custom");

    // Connect search
    connect(searchEdit, &QLineEdit::textChanged, this, &RetrievePage::populateTable);
    populateTable(); // Populate initially with all members
    connect(tableWidget, &QTableWidget::cellClicked, this, &RetrievePage::handleCellClick);
}

void RetrievePage::populateTable(const QString &filter)
{
    QVector<Member> allMembers = memberManager->getAllMembers();
    QVector<Member> filteredMembers;

    if (filter.trimmed().isEmpty())
    {
        filteredMembers = allMembers;
    }
    else
    {
        for (const Member &m : allMembers)
        {
            const User &u = userDataManager->getUserDataById(m.getUserId());
            if (u.getName().contains(filter, Qt::CaseInsensitive) ||
                u.getEmail().contains(filter, Qt::CaseInsensitive))
            {
                filteredMembers.append(m);
            }
        }
    }

    tableWidget->clearContents();
    tableWidget->setRowCount(filteredMembers.isEmpty() ? 1 : filteredMembers.size());

    if (filteredMembers.isEmpty())
    {
        tableWidget->setItem(0, 0, new QTableWidgetItem("-"));
        tableWidget->setItem(0, 1, new QTableWidgetItem("No data"));
        tableWidget->setItem(0, 2, new QTableWidgetItem("-"));
        return;
    }

    for (int i = 0; i < filteredMembers.size(); ++i)
    {
        const Member &m = filteredMembers[i];
        const User &u = userDataManager->getUserDataById(m.getUserId());

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(m.getId()));
        QTableWidgetItem *nameItem = new QTableWidgetItem(u.getName());
        QTableWidgetItem *emailItem = new QTableWidgetItem(u.getEmail());

        idItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setTextAlignment(Qt::AlignCenter);
        emailItem->setTextAlignment(Qt::AlignCenter);

        tableWidget->setItem(i, 0, idItem);
        tableWidget->setItem(i, 1, nameItem);
        tableWidget->setItem(i, 2, emailItem);

    }
}

void RetrievePage::showMessageDialog(const QString &message, bool isError)
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(isError ? tr("Error") : tr("Success"));
    dialog->setFixedSize(400, 240);
    dialog->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dialog->setAttribute(Qt::WA_TranslucentBackground);

    QWidget *container = new QWidget(dialog);
    container->setObjectName("messageContainer");
    container->setStyleSheet(QString(R"(
        QWidget#messageContainer {
            background: %1;
            border-radius: 16px;
            border: 1px solid %2;
        }
    )")
                                 .arg(
                                     isDarkTheme ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
                                     isDarkTheme ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"));

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setSpacing(24);
    layout->setContentsMargins(32, 32, 32, 32);

    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedSize(64, 64);
    iconLabel->setScaledContents(false);
    iconLabel->setAlignment(Qt::AlignCenter);

    QPixmap iconPixmap(isError ? ":/Images/error.png" : ":/Images/success.png");
    if (!iconPixmap.isNull())
    {
        iconPixmap = iconPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        iconLabel->setPixmap(iconPixmap);
    }

    QLabel *messageLabel = new QLabel(message);
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet(QString("color: %1; font-size: 16px; font-weight: 500;")
                                    .arg(isDarkTheme ? "#F9FAFB" : "#111827"));

    layout->addWidget(iconLabel, 0, Qt::AlignCenter);
    layout->addWidget(messageLabel);

    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(container);

    QPoint parentCenter = this->mapToGlobal(this->rect().center());
    dialog->move(parentCenter.x() - dialog->width() / 2,
                 parentCenter.y() - dialog->height() / 2);

    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect(dialog);
    container->setGraphicsEffect(opacityEffect);

    QPropertyAnimation *fadeIn = new QPropertyAnimation(opacityEffect, "opacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);

    QPropertyAnimation *fadeOut = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    QTimer *dismissTimer = new QTimer(dialog);
    dismissTimer->setSingleShot(true);
    connect(dismissTimer, &QTimer::timeout, [dialog, fadeOut]()
            { fadeOut->start(QAbstractAnimation::DeleteWhenStopped); });
    connect(fadeOut, &QPropertyAnimation::finished, dialog, &QDialog::accept);

    dialog->show();
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    dismissTimer->start(2000);

    dialog->exec();
    dialog->deleteLater();
}

void RetrievePage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    leftSidebar->updateTheme(isDark);

    if (const auto container = findChild<QWidget *>("cardContainer"))
    {
        container->setStyleSheet(QString(R"(
            QWidget#cardContainer {
                background: %1;
                border-radius: 24px;
                border: 1px solid %2;
            }
        )")
                                     .arg(
                                         isDark ? "rgba(31, 41, 55, 0.95)" : "rgba(255, 255, 255, 0.95)",
                                         isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)"));
    }

    // Update table widget style if it exists
    if (const auto table = findChild<QTableWidget *>("dataTable"))
    {
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
        )")
                                 .arg(isDark ? "#F9FAFB" : "#111827",
                                      isDark ? "#4B5563" : "#D1D5DB",
                                      isDarkTheme ? "rgba(55, 65, 81, 0.95)" : "rgba(229, 231, 235, 0.95)"));
    }
}

void RetrievePage::handlePageChange(const QString &pageId)
{
    if (pageId == "custom")
    {
        contentStack->setCurrentWidget(customContent);
        populateTable();
    }
    if (pageId == "add-member")
    {
        contentStack->setCurrentWidget(addMemberPage);
    }
}
void RetrievePage::handleCellClick(int row, int column)
{
    QTableWidgetItem* idItem = tableWidget->item(row, 0);
    if (!idItem) return;

    int memberId = idItem->text().toInt();

    const Member& m = memberManager->getMemberById(memberId);
    const User& u = userDataManager->getUserDataById(m.getUserId());

    QString details = QString("Name: %1\nEmail: %2\nClassId: %3\n\nWhat would you like to do?")
        .arg(u.getName())
        .arg(u.getEmail())
        .arg(m.getClassId());

    // Create a QMessageBox with custom buttons
    QMessageBox msgBox;
    msgBox.setWindowTitle("Member Options");
    msgBox.setText(details);
    msgBox.setIcon(QMessageBox::Question);

    QPushButton* renewButton = msgBox.addButton("Renew Subscription", QMessageBox::AcceptRole);
    QPushButton* cancelButton = msgBox.addButton("Cancel Subscription", QMessageBox::DestructiveRole);
    msgBox.addButton(QMessageBox::Close); // optional close button

    msgBox.exec();

    if (msgBox.clickedButton() == renewButton) {
        qDebug() << "Renew subscription for member ID:" << memberId;
        QString error2 = "renew error";
        memberManager->renewSubscription(memberId,m.getSubscription().getType(),m.getSubscription().isVIP(),error2);
    }
    else if (msgBox.clickedButton() == cancelButton) {
        qDebug() << "Cancel subscription for member ID:" << memberId;
        QString error = "cancelling error";
        memberManager->cancelSubscription(memberId,error);
    }
}


