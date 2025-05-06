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
#include <QComboBox>
#include <QGroupBox>
#include <QMap>

AvailableClassesScreen::AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent)
    : QWidget(parent), classDataManager(dataManager)
{
    setupCoaches();
    setupUI();
    refreshClasses();
}

AvailableClassesScreen::~AvailableClassesScreen()
{
}

void AvailableClassesScreen::setupCoaches()
{

    coaches.append(Staff(1, "John Smith", "john.smith@example.com", "password123", "john.jpg", QDate(1985, 5, 15), Staff::Role::COACH));
    coaches.append(Staff(2, "Sarah Johnson", "sarah.johnson@example.com", "password123", "sarah.jpg", QDate(1990, 8, 22), Staff::Role::COACH));
    coaches.append(Staff(3, "Michael Brown", "michael.brown@example.com", "password123", "michael.jpg", QDate(1982, 3, 10), Staff::Role::COACH));
    coaches.append(Staff(4, "Emily Davis", "emily.davis@example.com", "password123", "emily.jpg", QDate(1988, 12, 5), Staff::Role::COACH));
    coaches.append(Staff(5, "David Wilson", "david.wilson@example.com", "password123", "david.jpg", QDate(1979, 7, 18), Staff::Role::COACH));
}

void AvailableClassesScreen::setupUI()
{
    mainVLayout = new QVBoxLayout(this);
    mainVLayout->setContentsMargins(20, 20, 20, 20);
    mainVLayout->setSpacing(20);

    addClassButton = new QPushButton("Add Class");
    addClassButton->setStyleSheet(
        "background-color: #DFD0B8;"
        "color: black;"
        "border: none;"
        "padding: 10px 20px;"
        "font-size: 16px;"
        "border-radius: 5px;"
    );

    connect(addClassButton, &QPushButton::clicked, this, &AvailableClassesScreen::showAddClassDialog);

    mainVLayout->addWidget(addClassButton, 0, Qt::AlignRight);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border: none;");

    scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setSpacing(30);

    scrollArea->setWidget(scrollWidget);
    mainVLayout->addWidget(scrollArea);

    setLayout(mainVLayout);
}

void AvailableClassesScreen::refreshClasses()
{

    QWidget* contentWidget = scrollArea->widget();
    delete contentWidget;

    scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setSpacing(30);


    QVector<Class> classes = classDataManager->getAllClasses();

    QMap<QString, QVector<Class>> classesByCoach;


    for (const Class &gymClass : classes) {
        classesByCoach[gymClass.getCoachName()].append(gymClass);
    }


    for (const Staff &coach : coaches) {
        QString coachName = coach.getName();

        QVector<Class> coachClasses = classesByCoach.value(coachName);
        if (coachClasses.isEmpty() && classesByCoach.keys().contains(coachName) == false) {

            continue;
        }

        QGroupBox* coachGroup = new QGroupBox(coachName);
        coachGroup->setStyleSheet(
            "QGroupBox {"
            "   font-size: 18px;"
            "   font-weight: bold;"
            "   border: none;"
            "   margin-top: 15px;"
            "}"
            "QGroupBox::title {"
            "   subcontrol-origin: margin;"
            "   left: 10px;"
            "   padding: 0 5px;"
            "   color: #2c3e50;"
            "}"
        );


        QVBoxLayout* coachLayout = new QVBoxLayout(coachGroup);
        coachLayout->setContentsMargins(10, 25, 10, 10);
        coachLayout->setSpacing(20);


        if (classesByCoach.contains(coachName)) {
            QGridLayout* classesGrid = new QGridLayout();
            classesGrid->setSpacing(20);
            coachLayout->addLayout(classesGrid);


            int col = 0;
            int row = 0;
            for (const Class &gymClass : classesByCoach[coachName]) {
                QWidget *card = new QWidget;
                createClassCard(gymClass, classesGrid, row, col);


                col++;
                if (col >= 3) {
                    col = 0;
                    row++;
                }
            }


            for (int i = 0; i < 3; ++i) {
                classesGrid->setColumnStretch(i, 1);
            }
        } else {

            QLabel* noClassesLabel = new QLabel("No classes scheduled for this coach");
            noClassesLabel->setAlignment(Qt::AlignCenter);
            noClassesLabel->setStyleSheet("color: #666; font-style: italic;");
            coachLayout->addWidget(noClassesLabel);
        }


        scrollLayout->addWidget(coachGroup);
    }

    scrollArea->setWidget(scrollWidget);
}

void AvailableClassesScreen::createClassCard(const Class &gymClass, QGridLayout *classesGrid, int row, int col)
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
    classesGrid->addWidget(card, row, col);
}

void AvailableClassesScreen::showAddClassDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add New Class"));
    dialog.setMinimumWidth(400);

    QFormLayout form(&dialog);
    QLineEdit *classNameEdit = new QLineEdit;


    QComboBox *coachComboBox = new QComboBox;
    for (const Staff &coach : coaches) {
        coachComboBox->addItem(coach.getName(), coach.getId());
    }

    QDateEdit *fromDateEdit = new QDateEdit;
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDate(QDate::currentDate());
    fromDateEdit->setDisplayFormat("dd/MM/yyyy");

    QDateEdit *toDateEdit = new QDateEdit;
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDate(QDate::currentDate().addDays(7));
    toDateEdit->setDisplayFormat("dd/MM/yyyy");

    QSpinBox *capacitySpinBox = new QSpinBox;
    capacitySpinBox->setRange(1, 100);
    capacitySpinBox->setValue(20);

    form.addRow(tr("Class Name:"), classNameEdit);
    form.addRow(tr("Coach:"), coachComboBox);
    form.addRow(tr("Start Date:"), fromDateEdit);
    form.addRow(tr("End Date:"), toDateEdit);
    form.addRow(tr("Capacity:"), capacitySpinBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        if (classNameEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Class name cannot be empty!"));
            return;
        }

        if (fromDateEdit->date() > toDateEdit->date()) {
            QMessageBox::warning(this, tr("Error"), tr("End date cannot be before start date!"));
            return;
        }

        Class newClass;

        newClass.setClassName(classNameEdit->text().trimmed());
        newClass.setCoachName(coachComboBox->currentText());  // Use selected coach name
        newClass.setFromDate(fromDateEdit->date());
        newClass.setToDate(toDateEdit->date());
        newClass.setCapacity(capacitySpinBox->value());
        newClass.setNumOfEnrolled(0);

        qDebug() << "Adding new class with ID:" << newClass.getId()
                 << "Name:" << newClass.getClassName()
                 << "Coach:" << newClass.getCoachName();

        QString errorMessage;
        if (!classDataManager->addClass(newClass, errorMessage)) {
            QMessageBox::warning(this, tr("Error"),
                tr("Failed to add class: %1").arg(errorMessage));
            return;
        }

        refreshClasses();
        QMessageBox::information(this, tr("Success"),
            tr("Class '%1' added successfully!").arg(newClass.getClassName()));
    }
}

void AvailableClassesScreen::handleEnrollment(int classId)
{
    QMessageBox::information(this, tr("Enrollment"),
        tr("Enrollment for class ID %1 will be implemented").arg(classId));
}

void AvailableClassesScreen::handleWaitlist(int classId)
{
    QMessageBox::information(this, tr("Waitlist"),
        tr("Waitlist for class ID %1 will be implemented").arg(classId));
}