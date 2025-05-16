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
#include <QRandomGenerator>

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

void AvailableClassesScreen::setWorkoutDataManager(WorkoutDataManager* manager) {
    qDebug() << "Setting workout manager:" << manager;
    workoutManager = manager;
    
    // Refresh all pages with the new workout manager
    if (contentStack && contentStack->count() > 0) {
        qDebug() << "Refreshing all pages with new workout manager";
        
        // Update Classes page (index 0)
        QWidget* classesWidget = ClassesContent();
        QWidget* oldClassesWidget = contentStack->widget(0);
        contentStack->removeWidget(oldClassesWidget);
        oldClassesWidget->deleteLater();
        contentStack->insertWidget(0, classesWidget);
        
        // Update Workouts page (index 1)
        QWidget* workoutsWidget = WorkoutsContent();
        QWidget* oldWorkoutsWidget = contentStack->widget(1);
        contentStack->removeWidget(oldWorkoutsWidget);
        oldWorkoutsWidget->deleteLater();
        contentStack->insertWidget(1, workoutsWidget);
        
        // Update History page (index 2)
        QWidget* historyWidget = ExtraContent();
        QWidget* oldHistoryWidget = contentStack->widget(2);
        contentStack->removeWidget(oldHistoryWidget);
        oldHistoryWidget->deleteLater();
        contentStack->insertWidget(2, historyWidget);
        
        qDebug() << "All pages refreshed with new workout manager";
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
            qDebug() << "Loaded member data - Member ID:" << memberId << "Class ID:" << currentMember.getClassId();
        } else {
            currentMember = Member(0, currentUser.getId(), -1);
            qDebug() << "User is not a member";
        }
    }

    // Refresh all content when user data changes
    if (contentStack) {
        qDebug() << "Refreshing content after user data loaded";
        int currentIndex = contentStack->currentIndex();
        
        // Refresh workouts page (index 1)
        QWidget* workoutsWidget = WorkoutsContent();
        if (contentStack->count() > 1) {
            QWidget* oldWidget = contentStack->widget(1);
            contentStack->removeWidget(oldWidget);
            oldWidget->deleteLater();
        }
        contentStack->insertWidget(1, workoutsWidget);
        
        // Set current index back to what it was
        contentStack->setCurrentIndex(currentIndex);
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

    // Check if member is enrolled in any class
    if (currentMember.getClassId() > 0) {
        Class enrolledClass = classDataManager->getClassById(currentMember.getClassId());
        if (enrolledClass.getId() > 0) { // Check if class exists
            names << enrolledClass.getClassName();
            qDebug() << "Found enrolled class:" << enrolledClass.getClassName() << "with ID:" << enrolledClass.getId();
        }
    }

    // Also check class's enrolled members list
    for (const auto& gymClass : all) {
        if (gymClass.isMemberEnrolled(currentUser.getId()) && !names.contains(gymClass.getClassName())) {
            names << gymClass.getClassName();
            qDebug() << "Found additional enrollment in class:" << gymClass.getClassName() << "with ID:" << gymClass.getId();
        }
    }

    if (names.isEmpty()) {
        enrolledClassesLabel->setText("You're not enrolled in any classes yet.");
        qDebug() << "No enrolled classes found for user ID:" << currentUser.getId();
    } else {
        enrolledClassesLabel->setText(QString("Your classes: %1").arg(names.join(", ")));
        qDebug() << "Found enrolled classes:" << names;
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
    
    // Get member ID from user ID
    int memberId = -1;
    if (memberDataManager && currentUser.getId() > 0) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
    }
    
    // Check if this class is the member's enrolled class
    bool isEnrolled = (memberId > 0 && 
                      ((currentMember.getClassId() == gymClass.getId()) || 
                       gymClass.isMemberEnrolled(memberId)));

    qDebug() << "Class:" << gymClass.getClassName() << "ID:" << gymClass.getId() 
             << "Member ID:" << memberId 
             << "Current Member Class ID:" << currentMember.getClassId() 
             << "Is Enrolled:" << isEnrolled;

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
    // Get member ID from user ID
    int memberId = -1;
    if (memberDataManager && currentUser.getId() > 0) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
    }

    if (memberId <= 0) {
        NotificationManager::instance().showNotification(
            tr("Enrollment Failed"),
            "You must be a member to enroll in classes",
            nullptr,
            NotificationType::Error
        );
        return;
    }

    // Check if member is already enrolled in any class
    if (currentMember.getClassId() > 0) {
        Class currentClass = classDataManager->getClassById(currentMember.getClassId());
        if (currentClass.getId() > 0) {
            NotificationManager::instance().showNotification(
                tr("Enrollment Failed"),
                tr("You are already enrolled in \"%1\". Please unenroll from it first.").arg(currentClass.getClassName()),
                nullptr,
                NotificationType::Error
            );
            return;
        }
    }

    QString error;
    if (!classDataManager->enrollMember(classId, memberId, error)) {
        NotificationManager::instance().showNotification(
            tr("Enrollment Failed"),
            error.toUtf8().constData(),
            nullptr,
            NotificationType::Error
        );
    } else {
        // Update current member data after successful enrollment
        currentMember = memberDataManager->getMemberById(memberId);

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
    // Get member ID from user ID
    int memberId = -1;
    if (memberDataManager && currentUser.getId() > 0) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
    }

    if (memberId <= 0) {
        NotificationManager::instance().showNotification(
            tr("Unenroll Failed"),
            "You must be a member to unenroll from classes",
            nullptr,
            NotificationType::Error
        );
        return;
    }

    QString error;
    if (!classDataManager->unenrollMember(classId, memberId, error)) {
        NotificationManager::instance().showNotification(
            tr("Unenroll Failed"),
            error.toUtf8().constData(),
            nullptr,
            NotificationType::Error
        );
    } else {
        // Update current member data after successful unenrollment
        currentMember = memberDataManager->getMemberById(memberId);

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
QWidget* AvailableClassesScreen::WorkoutsContent() {
    qDebug() << "Starting WorkoutsContent() function";
    
    // Create main widget and layout
    QWidget* workoutsContent = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(workoutsContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Add title
    QLabel* titleLabel = new QLabel("Monthly Workout Schedule");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6647D8;");
    mainLayout->addWidget(titleLabel);

    // Create scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        "    background: rgba(220, 220, 255, 0.1);"
        "    width: 12px;"
        "    margin: 0px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(139, 92, 246, 0.3);"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
    );

    // Create content widget for scroll area
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(15);

    // Check if workout manager exists
    if (!workoutManager) {
        qDebug() << "ERROR: workoutManager is null!";
        QLabel* errorLabel = new QLabel("Workout system is currently unavailable.");
        errorLabel->setStyleSheet("color: red;");
        mainLayout->addWidget(errorLabel);
        return workoutsContent;
    }

    // Get all available workouts
    QVector<Workout> allWorkouts = workoutManager->getAllWorkouts();
    if (allWorkouts.isEmpty()) {
        qDebug() << "ERROR: No workouts found in the system!";
        QLabel* errorLabel = new QLabel("No workouts available.");
        errorLabel->setStyleSheet("color: red;");
        mainLayout->addWidget(errorLabel);
        return workoutsContent;
    }

    qDebug() << "Found" << allWorkouts.size() << "workouts in the system";

    // Get date range (today to end of month)
    QDate today = QDate::currentDate();
    QDate endOfMonth = QDate(today.year(), today.month(), today.daysInMonth());
    
    // For each day from today to end of month
    for (QDate date = today; date <= endOfMonth; date = date.addDays(1)) {
        qDebug() << "Processing date:" << date.toString();
        
        // Create a flag to check if we should skip this date
        bool skipDate = false;

        // Skip if workout is already completed for this date (only if user is logged in)
        if (currentUser.getId() > 0) {
            QVector<WorkoutLog> logs = workoutManager->getUserWorkoutLogsByDateRange(
                currentUser.getId(), date, date);
            if (!logs.isEmpty()) {
                qDebug() << "Workout already completed for" << date.toString();
                skipDate = true;
            }
        }

        // Continue to next date if we should skip this one
        if (skipDate) {
            continue;
        }

        // Select random workout for this day
        int randomIndex = QRandomGenerator::global()->bounded(allWorkouts.size());
        const Workout& workout = allWorkouts[randomIndex];
        qDebug() << "Selected workout:" << workout.name << "for date:" << date.toString();

        // Create workout card
        QWidget* card = new QWidget();
        card->setObjectName("workoutCard");
        card->setStyleSheet(
            "QWidget#workoutCard {"
            "    background-color: rgba(139, 92, 246, 0.05);"
            "    border: 1px solid #BFAEF5;"
            "    border-radius: 12px;"
            "    padding: 15px;"
            "}"
        );

        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        
        // Add date header
        QLabel* dateLabel = new QLabel(date.toString("dddd, MMMM d"));
        dateLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #6647D8;");
        cardLayout->addWidget(dateLabel);

        // Add workout details
        QLabel* workoutLabel = new QLabel(QString("<b>%1</b>").arg(workout.name));
        workoutLabel->setStyleSheet("font-size: 16px; color: #4B3C9C;");
        cardLayout->addWidget(workoutLabel);

        QLabel* detailsLabel = new QLabel(
            QString("Duration: %1 min | Calories: %2 | Difficulty: %3")
                .arg(workout.duration)
                .arg(workout.totalCalories)
                .arg(workout.difficulty)
        );
        detailsLabel->setStyleSheet("color: #666666;");
        cardLayout->addWidget(detailsLabel);

        // Add exercise list
        QString exerciseText = "<ul style='margin: 5px 0; color: #666666;'>";
        for (const Exercise& exercise : workout.exercises) {
            exerciseText += QString("<li>%1 - %2 sets x %3</li>")
                .arg(exercise.name)
                .arg(exercise.sets)
                .arg(exercise.reps);
        }
        exerciseText += "</ul>";
        
        QLabel* exercisesLabel = new QLabel(exerciseText);
        exercisesLabel->setTextFormat(Qt::RichText);
        cardLayout->addWidget(exercisesLabel);

        // Add train button (only if user is logged in)
        QPushButton* trainButton = new QPushButton(currentUser.getId() > 0 ? "Complete Workout" : "Log in to Train");
        trainButton->setCursor(Qt::PointingHandCursor);
        trainButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #81C784;"
            "    color: white;"
            "    border: none;"
            "    padding: 8px 16px;"
            "    border-radius: 4px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #66BB6A;"
            "}"
        );

        // Connect button to log workout
        connect(trainButton, &QPushButton::clicked, [this, workout, card, date]() {
            qDebug() << "Logging workout:" << workout.name << "for date:" << date.toString();
            
            if (!workoutManager || currentUser.getId() <= 0) {
                QMessageBox::warning(this, "Error", "Please log in to complete workouts.");
                return;
            }

            WorkoutLog log;
            log.userId = currentUser.getId();
            log.workoutId = workout.id;
            log.timestamp = QDateTime(date, QTime::currentTime());
            log.totalCaloriesBurnt = workout.totalCalories;
            log.classId = currentMember.getClassId();

            // Mark all exercises as completed
            for (const Exercise& exercise : workout.exercises) {
                log.completedExercises.append(qMakePair(exercise.name, true));
            }

            QString errorMessage;
            if (workoutManager->logWorkout(log, errorMessage)) {
                QMessageBox::information(this, "Success", 
                    QString("Completed workout: %1\nCalories burned: %2")
                    .arg(workout.name)
                    .arg(workout.totalCalories));
                
                // Remove the card
                card->hide();
                card->deleteLater();
            } else {
                QMessageBox::warning(this, "Error", 
                    QString("Failed to log workout: %1").arg(errorMessage));
            }
        });

        cardLayout->addWidget(trainButton);
        scrollLayout->addWidget(card);
    }

    // Add scroll area to main layout
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    qDebug() << "WorkoutsContent() function completed";
    return workoutsContent;
}
QWidget* AvailableClassesScreen::ExtraContent() {
    std::cout << "\n=== Starting ExtraContent() function ===" << std::endl;
    qDebug() << "Starting ExtraContent() function";

    // Create main widget and layout
    QWidget* historyContent = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(historyContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Add title
    QLabel* titleLabel = new QLabel("Workout History");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6647D8;");
    mainLayout->addWidget(titleLabel);

    // Check if workout manager exists
    std::cout << "Checking workout manager..." << std::endl;
    qDebug() << "Checking workout manager, pointer value:" << workoutManager;
    if (!workoutManager) {
        std::cout << "ERROR: workoutManager is null!" << std::endl;
        qDebug() << "ERROR: workoutManager is null!";
        QLabel* errorLabel = new QLabel("Workout system is currently unavailable.");
        errorLabel->setStyleSheet("color: red;");
        mainLayout->addWidget(errorLabel);
        return historyContent;
    }
    std::cout << "Workout manager is valid" << std::endl;

    // Check if user is logged in
    std::cout << "Checking user login status... User ID:" << currentUser.getId() << std::endl;
    qDebug() << "Current user ID:" << currentUser.getId();
    if (currentUser.getId() <= 0) {
        std::cout << "ERROR: User not logged in!" << std::endl;
        qDebug() << "ERROR: User not logged in!";
        QLabel* loginLabel = new QLabel("Please log in to view your workout history.");
        loginLabel->setStyleSheet("color: #666; font-size: 16px;");
        mainLayout->addWidget(loginLabel);
        return historyContent;
    }
    std::cout << "User is logged in with ID:" << currentUser.getId() << std::endl;

    // Get user's workout logs
    std::cout << "Fetching workout logs for user ID:" << currentUser.getId() << std::endl;
    QVector<WorkoutLog> userLogs = workoutManager->getUserWorkoutLogs(currentUser.getId());
    qDebug() << "Found" << userLogs.size() << "workout logs for user";
    std::cout << "Found " << userLogs.size() << " workout logs" << std::endl;

    // Create scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        "    background: rgba(220, 220, 255, 0.1);"
        "    width: 12px;"
        "    margin: 0px;"
        "    border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(139, 92, 246, 0.3);"
        "    min-height: 20px;"
        "    border-radius: 6px;"
        "}"
    );

    // Create content widget for scroll area
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(15);

    if (userLogs.isEmpty()) {
        std::cout << "No workout logs found for user" << std::endl;
        QLabel* noLogsLabel = new QLabel("No workout history found. Start training to see your progress!");
        noLogsLabel->setStyleSheet("color: #666; font-size: 16px;");
        noLogsLabel->setWordWrap(true);
        mainLayout->addWidget(noLogsLabel);
    } else {
        std::cout << "Creating statistics section..." << std::endl;
        // Add statistics section
        QWidget* statsCard = new QWidget();
        statsCard->setStyleSheet(
            "QWidget { background: rgba(102, 71, 216, 0.1); border-radius: 10px; padding: 15px; }"
        );
        QVBoxLayout* statsLayout = new QVBoxLayout(statsCard);
        
        // Total workouts completed
        int totalWorkouts = workoutManager->getTotalWorkoutsCompleted(currentUser.getId());
        std::cout << "Total workouts completed: " << totalWorkouts << std::endl;
        QLabel* totalWorkoutsLabel = new QLabel(QString("Total Workouts Completed: %1").arg(totalWorkouts));
        totalWorkoutsLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #6647D8;");
        statsLayout->addWidget(totalWorkoutsLabel);
        
        // Total calories burnt
        int totalCalories = workoutManager->getTotalCaloriesBurnt(currentUser.getId());
        std::cout << "Total calories burnt: " << totalCalories << std::endl;
        QLabel* totalCaloriesLabel = new QLabel(QString("Total Calories Burnt: %1").arg(totalCalories));
        totalCaloriesLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #6647D8;");
        statsLayout->addWidget(totalCaloriesLabel);
        
        scrollLayout->addWidget(statsCard);

        // Add workout history cards
        std::cout << "Creating workout history cards..." << std::endl;
        for (const WorkoutLog& log : userLogs) {
            std::cout << "Creating card for workout ID: " << log.workoutId << std::endl;
            QWidget* card = new QWidget();
            card->setStyleSheet(
                "QWidget { background: white; border-radius: 10px; padding: 15px; }"
                "QWidget:hover { background: #f8f8ff; }"
            );
            QVBoxLayout* cardLayout = new QVBoxLayout(card);

            // Get workout details
            Workout workout = workoutManager->getWorkoutById(log.workoutId);
            std::cout << "Workout name: " << workout.name.toStdString() << std::endl;
            
            // Workout name and date
            QLabel* workoutLabel = new QLabel(QString("<b>%1</b>").arg(workout.name));
            workoutLabel->setStyleSheet("font-size: 18px; color: #333;");
            cardLayout->addWidget(workoutLabel);

            QLabel* dateLabel = new QLabel(log.timestamp.toString("MMMM d, yyyy 'at' h:mm ap"));
            dateLabel->setStyleSheet("color: #666; margin-bottom: 10px;");
            cardLayout->addWidget(dateLabel);

            // Calories burnt
            QLabel* caloriesLabel = new QLabel(QString("Calories Burnt: %1").arg(log.totalCaloriesBurnt));
            caloriesLabel->setStyleSheet("color: #6647D8; font-weight: bold;");
            cardLayout->addWidget(caloriesLabel);

            // Completed exercises
            QLabel* exercisesLabel = new QLabel("Completed Exercises:");
            exercisesLabel->setStyleSheet("color: #333; margin-top: 10px;");
            cardLayout->addWidget(exercisesLabel);

            for (const auto& exercise : log.completedExercises) {
                QString status = exercise.second ? "✅" : "❌";
                QLabel* exerciseLabel = new QLabel(QString("%1 %2").arg(status, exercise.first));
                exerciseLabel->setStyleSheet("color: #555; margin-left: 20px;");
                cardLayout->addWidget(exerciseLabel);
            }

            scrollLayout->addWidget(card);
        }
    }

    // Add scroll area to main layout
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    std::cout << "=== ExtraContent() function completed ===" << std::endl;
    qDebug() << "ExtraContent() function completed";
    return historyContent;
}
void AvailableClassesScreen::handlePageChange(const QString& pageID) {
    qDebug() << "Handling page change to:" << pageID;
    qDebug() << "Current workoutManager:" << workoutManager;

    // First, ensure all pages are up to date
    if (contentStack->count() > 0) {
        // Update Classes page (index 0)
        QWidget* classesWidget = ClassesContent();
        QWidget* oldClassesWidget = contentStack->widget(0);
        contentStack->removeWidget(oldClassesWidget);
        oldClassesWidget->deleteLater();
        contentStack->insertWidget(0, classesWidget);
        
        // Update Workouts page (index 1)
        QWidget* workoutsWidget = WorkoutsContent();
        QWidget* oldWorkoutsWidget = contentStack->widget(1);
        contentStack->removeWidget(oldWorkoutsWidget);
        oldWorkoutsWidget->deleteLater();
        contentStack->insertWidget(1, workoutsWidget);
        
        // Update History page (index 2)
        QWidget* historyWidget = ExtraContent();
        QWidget* oldHistoryWidget = contentStack->widget(2);
        contentStack->removeWidget(oldHistoryWidget);
        oldHistoryWidget->deleteLater();
        contentStack->insertWidget(2, historyWidget);
    }

    // Then change to the requested page
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

