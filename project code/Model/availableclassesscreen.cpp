#include "availableclassesscreen.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QProgressBar>

AvailableClassesScreen::AvailableClassesScreen(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AvailableClassesScreen::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    addClassButton = new QPushButton("Add Class");
    addClassButton->setStyleSheet(
        "background-color: #DFD0B8;"
        "color: black;"
        "border: none;"
        "padding: 10px 20px;"
        "font-size: 16px;"
        "border-radius: 5px;"
    );

    // Connect using lambda instead of slot
    connect(addClassButton, &QPushButton::clicked, [this]() {
        showAddClassDialog();
    });

    mainLayout->addWidget(addClassButton, 0, Qt::AlignRight);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: none;");

    scrollWidget = new QWidget;
    classesGridLayout = new QGridLayout(scrollWidget);
    classesGridLayout->setAlignment(Qt::AlignTop);
    classesGridLayout->setSpacing(20);
    scrollWidget->setLayout(classesGridLayout);

    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);

    setLayout(mainLayout);
}
void AvailableClassesScreen::addClass(const Class &gymClass)
{
    classes.append(gymClass);
    updateClassesDisplay();
}

void AvailableClassesScreen::createClassCard(const Class &gymClass)
{
    QWidget *card = new QWidget;
    card->setMinimumWidth(250);
    card->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    QString cardStyle = QString(
        "QWidget {"
        "   background-color: #DFD0B8;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "   border: 1px solid #393E46;"
        "}"
        "QLabel {"
        "   font-size: 14px;"
        "   color: black;"
        "}"
        "QLabel#title {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "}"
    );
    card->setStyleSheet(cardStyle);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(12);
    cardLayout->setContentsMargins(15, 15, 15, 15);

    QLabel *titleLabel = new QLabel(gymClass.getClassName());
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    QLabel *coachLabel = new QLabel(tr("<b>Coach:</b> ") + gymClass.getCoachName());
    coachLabel->setTextFormat(Qt::RichText);

    QLabel *timeLabel = new QLabel(tr("<b>Date:</b> %1 - %2").arg(
        gymClass.getFromDate().toString(QLocale::system().dateFormat(QLocale::ShortFormat)),
        gymClass.getToDate().toString(QLocale::system().dateFormat(QLocale::ShortFormat))));
    timeLabel->setTextFormat(Qt::RichText);

    QString capacityText = QString("%1 / %2").arg(gymClass.getNumOfEnrolled()).arg(gymClass.getCapacity());
    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0, gymClass.getCapacity());
    progressBar->setValue(gymClass.getNumOfEnrolled());
    progressBar->setTextVisible(true);
    progressBar->setFormat(capacityText);
    progressBar->setAlignment(Qt::AlignCenter);

    int enrollmentPercentage = (gymClass.getNumOfEnrolled() * 100) / gymClass.getCapacity();
    QString progressColor = enrollmentPercentage < 50 ? "#4CAF50" :
                          (enrollmentPercentage < 80 ? "#FFC107" : "#F44336");

    progressBar->setStyleSheet(QString(
        "QProgressBar {"
        "   border: 1px solid #393E46;"
        "   border-radius: 5px;"
        "   text-align: center;"
        "   height: 20px;"
        "   color: black;"
        "}"
        "QProgressBar::chunk {"
        "   background-color: %1;"
        "   border-radius: 5px;"
        "}"
    ).arg(progressColor));

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(coachLabel);
    cardLayout->addWidget(timeLabel);
    cardLayout->addWidget(progressBar);

    QPushButton *enrollButton = new QPushButton("Enroll");
    QPushButton *waitlistButton = new QPushButton("Waitlist");

    enrollButton->setStyleSheet("background-color: #948979; color: black; padding: 8px; border-radius: 4px;");
    waitlistButton->setStyleSheet("background-color: #948979; color: black; padding: 8px; border-radius: 4px;");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(enrollButton);
    buttonLayout->addWidget(waitlistButton);

    connect(enrollButton, &QPushButton::clicked, [this, classId = gymClass.getId()]() {
        handleEnrollment(classId);
    });
    connect(waitlistButton, &QPushButton::clicked, [this, classId = gymClass.getId()]() {
        handleWaitlist(classId);
    });

    cardLayout->addLayout(buttonLayout);
    classesGridLayout->addWidget(card, classesGridLayout->count() / 3, classesGridLayout->count() % 3);
}

void AvailableClassesScreen::updateClassesDisplay()
{
    QLayoutItem *item;
    while ((item = classesGridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const Class &gymClass : classes) {
        createClassCard(gymClass);
    }

    for (int i = 0; i < 3; ++i) {
        classesGridLayout->setColumnStretch(i, 1);
    }
}
void AvailableClassesScreen::showAddClassDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add New Class"));

    QFormLayout form(&dialog);

    QLineEdit *classNameEdit = new QLineEdit;
    QLineEdit *coachNameEdit = new QLineEdit;
    QDateEdit *fromDateEdit = new QDateEdit;
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDate(QDate::currentDate());
    QDateEdit *toDateEdit = new QDateEdit;
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDate(QDate::currentDate());
    QSpinBox *capacitySpinBox = new QSpinBox;
    capacitySpinBox->setRange(1, 100);
    capacitySpinBox->setValue(20);
    form.addRow(tr("Class Name:"), classNameEdit);
    form.addRow(tr("Coach Name:"), coachNameEdit);
    form.addRow(tr("Start Date:"), fromDateEdit);
    form.addRow(tr("End Date:"), toDateEdit);
    form.addRow(tr("Capacity:"), capacitySpinBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (classNameEdit->text().isEmpty() || coachNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("why empty :((("));
            return;
        }

        if (fromDateEdit->date() > toDateEdit->date()) {
            QMessageBox::warning(this, tr("Error"), tr("Ezay el nehaya abl el bedaya ._. "));
            return;
        }

        Class newClass;
        newClass.setClassName(classNameEdit->text());
        newClass.setCoachName(coachNameEdit->text());
        newClass.setFromDate(fromDateEdit->date());
        newClass.setToDate(toDateEdit->date());
        newClass.setCapacity(capacitySpinBox->value());
        newClass.setNumOfEnrolled(0);
        newClass.setId(classes.size() + 1);

        addClass(newClass);
    }
}