// AvailableClassesScreen.cpp, CHANGE THIS LATER TO BE THE MAIN GYM SCREEN
//                                EACH PAGE SHOULD HAVE ITS OWN .CPP

#include "availableclassesscreen.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QProgressBar>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QHBoxLayout>

#include "Widgets/Notifications/NotificationManager.h"

AvailableClassesScreen::AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent)
    : QWidget(parent), classDataManager(dataManager)
{
    //dummy current user
    currentUser = User("Alice Example", "alice@example.com", "passw0rd", ":/avatars/alice.png", QDate(1995,6,1));
    currentUser.setId(1);
    currentMember = Member(1, currentUser.getId(), -1);

    setupCoaches();
    setupUI();
    refreshClasses();
}

AvailableClassesScreen::~AvailableClassesScreen() = default;

void AvailableClassesScreen::setupCoaches()
{
    coaches.append({1, "John Smith", "john.smith@example.com", "password123", "john.jpg", QDate(1985,5,15), Staff::Role::COACH});
    coaches.append({2, "Sarah Johnson", "sarah.johnson@example.com", "password123", "sarah.jpg", QDate(1990,8,22), Staff::Role::COACH});
    coaches.append({3, "Michael Brown", "michael.brown@example.com", "password123", "michael.jpg", QDate(1982,3,10), Staff::Role::COACH});
    coaches.append({4, "Emily Davis", "emily.davis@example.com", "password123", "emily.jpg", QDate(1988,12,5), Staff::Role::COACH});
    coaches.append({5, "David Wilson", "david.wilson@example.com", "password123", "david.jpg", QDate(1979,7,18), Staff::Role::COACH});
}

void AvailableClassesScreen::setupUI()
{
    //main horizontal layout (leftsidebar + contentStack)
    mainHLayout = new QHBoxLayout(this);
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->setSpacing(0);

    //create and define sidebar buttons (also it's initial theme)
    leftSidebar = new LeftSidebar();
    leftSidebar->addButton(":/Images/whistle.png", "Gym Classes", "classes");
    leftSidebar->addButton(":/Images/muscle.png", "Workouts", "workouts");
    leftSidebar->addButton(":/Images/team.png", "Add Class", "add-classes");
    leftSidebar->updateTheme(true); //HAS TO BE RE-IMPLEMENTED WHEN ADDING DARK THEME TO THIS PAGE (ALWAYS SET TO TRUE FOR NOW)
    mainHLayout->addWidget(leftSidebar);
    //page change handling
    connect(leftSidebar, &LeftSidebar::pageChanged, this, &AvailableClassesScreen::handlePageChange);


    //Content stack that holds pages
    contentStack = new QStackedWidget();
    contentStack->addWidget(ClassesContent()); // Add Gym Classes page
    contentStack->addWidget(WorkoutsContent()); // Add Workouts page
    contentStack->addWidget(ExtraContent()); // Add Workouts page
    mainHLayout->addWidget(contentStack);

    //initialize the state of side bar to the first button
    leftSidebar->setActiveButton("classes");

    setLayout(mainHLayout);
}

void AvailableClassesScreen::refreshClasses()
{
    auto all = classDataManager->getAllClasses();
    QStringList names;
    for (auto &c : all) {
        if (classDataManager->getClassById(c.getId())
            .isMemberEnrolled(currentUser.getId()))
        {
            names << c.getClassName();
        }
    }
    if (names.isEmpty())
        enrolledClassesLabel->setText("You’re not enrolled in any classes yet.");
    else
        enrolledClassesLabel->setText(QString("Your classes: %1").arg(names.join(", ")));

    delete scrollArea->widget();
    scrollWidget = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setSpacing(30);

    QMap<QString,QVector<Class>> byCoach;
    for (auto &c : all) byCoach[c.getCoachName()].append(c);

    for (auto &coach : coaches) {
        auto classesFor = byCoach.value(coach.getName());
        if (classesFor.isEmpty()) continue;

        QGroupBox* group = new QGroupBox(coach.getName());
        group->setStyleSheet(
            "QGroupBox { font-size:18px; font-weight:bold; border:none; margin-top:15px; }"
            "QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 5px; color:#2c3e50; }"
        );
        QVBoxLayout* glay = new QVBoxLayout(group);
        glay->setContentsMargins(10,25,10,10);
        glay->setSpacing(20);

        QGridLayout* grid = new QGridLayout;
        grid->setSpacing(20);
        glay->addLayout(grid);

        int row=0, col=0;
        for (auto &cls : classesFor) {
            createClassCard(cls, grid, row, col);
            if (++col >= 3) { col=0; ++row; }
        }
        for (int i=0;i<3;i++) grid->setColumnStretch(i,1);

        scrollLayout->addWidget(group);
    }

    scrollArea->setWidget(scrollWidget);
}

void AvailableClassesScreen::createClassCard(const Class &gymClass,
                                             QGridLayout *classesGrid,
                                             int row, int col)
{
    QWidget *card = new QWidget;
    card->setMinimumWidth(250);
    card->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    card->setStyleSheet(
        "QWidget { background-color:#DFD0B8; border-radius:10px; padding:15px; border:1px solid #393E46; }"
        "QLabel { font-size:14px; color:black; }"
        "QLabel#title { font-size:18px; font-weight:bold; color:#2c3e50; }"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(12);
    cardLayout->setContentsMargins(15,15,15,15);

    QLabel *titleLabel = new QLabel(gymClass.getClassName());
    titleLabel->setObjectName("title");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setWordWrap(true);

    QLabel *timeLabel = new QLabel(
        tr("<b>Date:</b> %1 - %2")
          .arg(gymClass.getFromDate().toString("dd/MM/yyyy"),
               gymClass.getToDate().toString("dd/MM/yyyy"))
    );
    timeLabel->setTextFormat(Qt::RichText);

    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0, gymClass.getCapacity());
    progressBar->setValue(gymClass.getNumOfEnrolled());
    progressBar->setFormat(QString("%1 / %2")
                              .arg(gymClass.getNumOfEnrolled())
                              .arg(gymClass.getCapacity()));
    progressBar->setAlignment(Qt::AlignCenter);
    int pct = gymClass.getNumOfEnrolled() * 100 / gymClass.getCapacity();
    QString chunkColor = pct<50 ? "#4CAF50" : (pct<80 ? "#FFC107" : "#F44336");
    progressBar->setStyleSheet(QString(
        "QProgressBar { border:1px solid #393E46; border-radius:5px; height:20px; }"
        "QProgressBar::chunk { background-color:%1; border-radius:5px; }"
    ).arg(chunkColor));

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(timeLabel);
    cardLayout->addWidget(progressBar);

    bool isFull = classDataManager->isClassFull(gymClass.getId());
    bool isEnrolled = classDataManager
                         ->getClassById(gymClass.getId())
                         .isMemberEnrolled(currentUser.getId());

    QPushButton *actionBtn = new QPushButton;
    actionBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    actionBtn->setMinimumHeight(30);

    if (isEnrolled) {
        actionBtn->setText("Cancel");
        actionBtn->setStyleSheet("background-color:#E57373; color:white; border-radius:4px; font-weight:bold;");
        connect(actionBtn, &QPushButton::clicked, [=]() {
            handleUnenroll(gymClass.getId());
        });
    } else {
        actionBtn->setText(isFull ? "Full" : "Enroll");
        actionBtn->setStyleSheet(QString(
            "background-color:%1; color:white; border-radius:4px; font-weight:bold;"
        ).arg(isFull ? "#B0BEC5" : "#81C784"));

        connect(actionBtn, &QPushButton::clicked, [=]() {
            if (isFull) {
                if (QMessageBox::question(
                        this, tr("Class Full"),
                        tr("This class is full. Join the waitlist?"),
                        QMessageBox::Yes|QMessageBox::No
                    ) == QMessageBox::Yes)
                {
                    handleWaitlist(gymClass.getId());
                }
            } else {
                handleEnrollment(gymClass.getId());
            }
        });
    }

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0,0,0,0);
    btnLayout->addWidget(actionBtn);
    cardLayout->addLayout(btnLayout);

    classesGrid->addWidget(card, row, col);
}

void AvailableClassesScreen::handleEnrollment(int classId)
{
    QString error;
    if (!classDataManager->enrollMember(classId, currentUser.getId(), error)) {

        NotificationManager::instance().showNotification(
            tr("Enrollment Failed"),
            error.toUtf8().constData(),
            nullptr,
            NotificationType::Error
        );
    } else {

        QString className = classDataManager->getClassById(classId).getClassName();
        NotificationManager::instance().showNotification(
            tr("Enrolled"),
            tr("Successfully enrolled in \"%1\".").arg(className),
            nullptr,
            NotificationType::Success
        );
    }
    refreshClasses();
}

void AvailableClassesScreen::handleUnenroll(int classId)
{
    QString error;
    if (!classDataManager->unenrollMember(classId, currentUser.getId(), error)) {
        NotificationManager::instance().showNotification(
            tr("Unenroll Failed"),
            error.toUtf8().constData(),
            nullptr,
            NotificationType::Error
        );
    } else {
        QString className = classDataManager->getClassById(classId).getClassName();
        NotificationManager::instance().showNotification(
            tr("Cancelled"),
            tr("Unenrolled from \"%1\".").arg(className),
            nullptr,
            NotificationType::Success
        );
    }
    refreshClasses();
}

void AvailableClassesScreen::handleWaitlist(int classId)
{
    QString error;
    if (!classDataManager->addToWaitlist(classId, currentUser.getId(), false, error)) {
        NotificationManager::instance().showNotification(
            tr("Waitlist Failed"),
            error.toUtf8().constData(),
            nullptr,
            NotificationType::Error
        );
    } else {
        QString className = classDataManager->getClassById(classId).getClassName();
        NotificationManager::instance().showNotification(
            tr("Waitlist"),
            tr("You’ve been added to the waitlist for \"%1\".").arg(className),
            nullptr,
            NotificationType::Success
        );
    }
    refreshClasses();
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
QWidget* AvailableClassesScreen::ClassesContent() {
    QWidget* classesContent = new QWidget(); //main widget that will be used in extrenal main layout
    QVBoxLayout* classesLayout= new QVBoxLayout(classesContent); //VBOX that will contain contents (added to the above widget)
    classesLayout->setContentsMargins(20,20,20,20);
    classesLayout->setSpacing(15);

    userNameLabel = new QLabel(QString("Hello, %1!").arg(currentUser.getName()));
    userNameLabel->setStyleSheet("font-size:20px; font-weight:bold;");
    enrolledClassesLabel = new QLabel;
    enrolledClassesLabel->setWordWrap(true);
    enrolledClassesLabel->setStyleSheet("color:#555;");

    QVBoxLayout* headerLayout = new QVBoxLayout;
    headerLayout->addWidget(userNameLabel);
    headerLayout->addWidget(enrolledClassesLabel);
    classesLayout->addLayout(headerLayout);

    addClassButton = new QPushButton("Add Class");
    addClassButton->setStyleSheet(
        "background-color:#DFD0B8; color:black; border:none; padding:10px 20px; "
        "font-size:16px; border-radius:5px;"
    );
    connect(addClassButton, &QPushButton::clicked, this, &AvailableClassesScreen::showAddClassDialog);
    classesLayout->addWidget(addClassButton, 0, Qt::AlignRight);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("border:none;");
    scrollWidget = new QWidget;
    scrollArea->setWidget(scrollWidget);
    classesLayout->addWidget(scrollArea);

    return classesContent; //returns the widget (page) pointer to be added to stacked widgets
}
//EXAMPLE FUNCTION FOR A SECOND PAGE (CHANGE LATER)
QWidget* AvailableClassesScreen::WorkoutsContent() {
    QWidget* workoutsContent = new QWidget();
    QLabel* workoutsLabel = new QLabel("THIS IS WORKOUTS PAGE (ADD YOUR PAGE LATER)", workoutsContent);
    return workoutsContent;
}
QWidget* AvailableClassesScreen::ExtraContent() {
    QWidget* Content = new QWidget();
    QLabel* Label = new QLabel("THIS IS AN EXTRA PAGE (ADD YOUR PAGE LATER)", Content);
    return Content;

}
void AvailableClassesScreen::handlePageChange(const QString& pageID) {
    if (pageID == "classes") {
        contentStack->setCurrentIndex(0);
    }
    else if (pageID == "workouts") {
        contentStack->setCurrentIndex(1);
    }
    else if (pageID == "add-classes") {
        contentStack->setCurrentIndex(2);
    }
}

