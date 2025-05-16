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
#include <QMessageBox>
#include <QProgressBar>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include "Widgets/Notifications/NotificationManager.h"
#include "Widgets/WorkoutProgressPage.h"

AvailableClassesScreen::AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent)
    : QWidget(parent), classDataManager(dataManager), userDataManager(nullptr), memberDataManager(nullptr),
      workoutManager(nullptr), isDarkTheme(true)
{
    setupCoaches();
    setupUI();
    refreshClasses();
}

AvailableClassesScreen::~AvailableClassesScreen() = default;

void AvailableClassesScreen::setCurrentUserEmail(const QString& email)
{
    currentUserEmail = email;
    loadUserData();
}

void AvailableClassesScreen::setUserDataManager(UserDataManager* manager)
{
    userDataManager = manager;
    if (!currentUserEmail.isEmpty() && userDataManager) {
        loadUserData();
    }
}

void AvailableClassesScreen::setMemberDataManager(MemberDataManager* manager)
{
    memberDataManager = manager;
    if (!currentUserEmail.isEmpty() && userDataManager && memberDataManager) {
        loadUserData();
    }
}

void AvailableClassesScreen::loadUserData()
{
    if (currentUserEmail.isEmpty() || !userDataManager) {
        return;
    }
    currentUser = userDataManager->getUserData(currentUserEmail);

    if (currentUser.getId() <= 0) {
        return;
    }
    
    if (userNameLabel) {
        userNameLabel->setText(QString("Hello, %1!").arg(currentUser.getName()));
    }
    
    if (memberDataManager) {
        bool isMember = memberDataManager->userIsMember(currentUser.getId());
        if (isMember) {
            int memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
            currentMember = memberDataManager->getMemberById(memberId);
        } else {
            currentMember = Member(0, currentUser.getId(), -1);
        }
    }

    updateTheme(isDarkTheme);
    refreshClasses();
}

void AvailableClassesScreen::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Update text color based on theme
    QString userNameColor = isDark ? "#FFFFFF" : "#333333";
    QString enrolledClassColor = isDark ? "#AAAAAA" : "#555555";
    
    if (userNameLabel) {
        userNameLabel->setStyleSheet(QString("font-size:20px; font-weight:bold; color:%1;").arg(userNameColor));
    }
    
    if (enrolledClassesLabel) {
        enrolledClassesLabel->setStyleSheet(QString("color:%1;").arg(enrolledClassColor));
    }

    QString coachTitleColor = isDark ? "#A78CF6" : "#6647D8";
    QString groupBoxStyle = QString(
        "QGroupBox { font-size:18px; font-weight:bold; border:none; margin-top:15px; }"
        "QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 5px; color:%1; }"
    ).arg(coachTitleColor);
    QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    for (QGroupBox* box : groupBoxes) {
        box->setStyleSheet(groupBoxStyle);
    }
}

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
    leftSidebar->addButton(":/Images/dumbbell.png", "Workouts", "workouts");
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
    if (currentUser.getId() <= 0) {
        if (enrolledClassesLabel) {
            enrolledClassesLabel->setText("Please log in to view your classes.");
        }
        return;
    }

    auto all = classDataManager->getAllClasses();
    QStringList names;
    for (auto &c : all) {
        if (classDataManager->getClassById(c.getId())
            .isMemberEnrolled(currentUser.getId()))
        {
            names << c.getClassName();
        }
    }
    
    if (names.isEmpty()) {
        enrolledClassesLabel->setText("You're not enrolled in any classes yet.");
    } else {
        enrolledClassesLabel->setText(QString("Your classes: %1").arg(names.join(", ")));
    }

    if (scrollArea && scrollArea->widget()) {
        delete scrollArea->widget();
    }
    
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
        QString groupBoxStyle = QString(
            "QGroupBox { font-size:18px; font-weight:bold; border:none; margin-top:15px; }"
            "QGroupBox::title { subcontrol-origin:margin; left:10px; padding:0 5px; color:%1; }"
        ).arg(isDarkTheme ? "#A78CF6" : "#6647D8");
        
        group->setStyleSheet(groupBoxStyle);
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
    const QString cardStyle = QString(R"(
    QWidget {
        background-color: rgba(139, 92, 246, 0.05); /* Soft lavender */
        border-radius: 12px;
        padding: 16px;
        border: 1px solid #BFAEF5; /* Muted lavender */
    }
    QLabel {
        font-size: 14px;
        color: #978ADD; /* Dark neutral gray - readable on white & lavender */
    }
    QLabel#title {
        font-size: 18px;
        font-weight: bold;
        color: #4B3C9C; /* Deep violet-blue - readable on all */
    }
    )");
    card->setStyleSheet(cardStyle);
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
        "QProgressBar { border:1px solid #BFAEF5; border-radius:12px; height:20px; color: #978ADD}"
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
    actionBtn->setCursor(Qt::PointingHandCursor);

    if (isEnrolled) {
        actionBtn->setText("Cancel");
        connect(actionBtn, &QPushButton::clicked, [=]() {
            handleUnenroll(gymClass.getId());
        });
        actionBtn->setStyleSheet(QString(
        "QPushButton {"
        " background-color: #E57373;"
        " color: white;"
        " border-radius: 4px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: #D36464;"
        "}"));

    } else {
        actionBtn->setText(isFull ? "Full" : "Enroll");
        actionBtn->setStyleSheet(QString(
        "QPushButton {"
        " background-color: %1;"
        " color: white;"
        " border-radius: 4px;"
        " font-weight: bold;"
        "}"
        "QPushButton:hover {"
        " background-color: %2;"
        "}"
        ).arg(isFull ? "#B0BEC5" : "#81C784", isFull ? "#839AA5" : "#56AB59"));


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
            tr("You've been added to the waitlist for \"%1\".").arg(className),
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
    scrollArea->setStyleSheet(QString(R"(
    QScrollArea {
        background: transparent;
    }
    QScrollBar:vertical {
        background: rgba(220, 220, 255, 0.1);
        width: 12px;
        margin: 0px;
        border-radius: 6px;
    }
    QScrollBar::handle:vertical {
        background: rgba(139, 92, 246, 0.3);
        min-height: 20px;
        border-radius: 6px;
    }
    QScrollBar::handle:vertical:hover {
        background: rgba(139, 92, 246, 0.5);
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
        background: none;
    }

    QScrollBar:horizontal {
        background: rgba(220, 220, 255, 0.1);
        height: 12px;
        margin: 0px;
        border-radius: 6px;
    }
    QScrollBar::handle:horizontal {
        background: rgba(139, 92, 246, 0.3);
        min-width: 20px;
        border-radius: 6px;
    }
    QScrollBar::handle:horizontal:hover {
        background: rgba(139, 92, 246, 0.5);
    }
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0px;
    }
    QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
        background: none;
    }
    )"));
    scrollWidget = new QWidget;
    scrollArea->setWidget(scrollWidget);
    classesLayout->addWidget(scrollArea);

    return classesContent; //returns the widget (page) pointer to be added to stacked widgets
}
//EXAMPLE FUNCTION FOR A SECOND PAGE (CHANGE LATER)
QWidget* AvailableClassesScreen::WorkoutsContent() {
    auto* workoutPage = new WorkoutProgressPage();
    workoutPage->setWorkoutDataManager(workoutManager);
    workoutPage->setClassDataManager(classDataManager);
    workoutPage->setUserId(currentUser.getId());
    workoutPage->updateTheme(isDarkTheme);
    return workoutPage;
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

