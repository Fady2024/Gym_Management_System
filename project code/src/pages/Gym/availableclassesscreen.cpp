
#include "availableclassesscreen.h"
#include <QGraphicsDropShadowEffect>
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
#include <QTimer>

#include "Widgets/Notifications/NotificationManager.h"
#include "Widgets/WorkoutProgressPage.h"

AvailableClassesScreen::AvailableClassesScreen(ClassDataManager* dataManager, QWidget *parent)
    : QWidget(parent), classDataManager(dataManager), userDataManager(nullptr), memberDataManager(nullptr),
      workoutManager(nullptr), isDarkTheme(true)
{
    qDebug() << "Initializing AvailableClassesScreen";
    setupCoaches();
    setupUI();
    
    // Explicitly ensure we're showing the classes page AFTER UI setup
    QTimer::singleShot(0, this, [this]() {
        std::cout << "\n=== Delayed Initialization Start ===" << std::endl;
        qDebug() << "Delayed initialization - forcing classes page";

        // Force a complete rebuild of the classes page
        std::cout << "Creating new classes widget" << std::endl;
        QWidget* classesWidget = ClassesContent();

        std::cout << "Removing old classes widget" << std::endl;
        QWidget* oldClassesWidget = contentStack->widget(0);
        contentStack->removeWidget(oldClassesWidget);
        oldClassesWidget->deleteLater();

        std::cout << "Inserting new classes widget" << std::endl;
        contentStack->insertWidget(0, classesWidget);

        std::cout << "Setting current index to 0" << std::endl;
        contentStack->setCurrentIndex(0);  // Force classes page
        leftSidebar->setActiveButton("classes");

        if (contentStack->currentWidget()) {
            std::cout << "Updating current widget" << std::endl;
            contentStack->currentWidget()->update();
        }

        std::cout << "Refreshing classes" << std::endl;
        refreshClasses();

        std::cout << "Final update" << std::endl;
        update();

        std::cout << "Final content stack index: " << contentStack->currentIndex() << std::endl;
        std::cout << "Final widget class name: " << contentStack->currentWidget()->metaObject()->className() << std::endl;
        std::cout << "=== Delayed Initialization Complete ===\n" << std::endl;
    });

    std::cout << "=== AvailableClassesScreen Constructor Complete ===\n" << std::endl;
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

    // Force refresh the current page to update all colors
    if (contentStack) {
        int currentIndex = contentStack->currentIndex();
        QWidget* newWidget = nullptr;

        switch (currentIndex) {
            case 0:
                newWidget = ClassesContent();
                break;
            case 1:
                newWidget = WorkoutsContent();
                break;
            case 2:
                newWidget = ExtraContent();
                break;
        }

        if (newWidget) {
            QWidget* oldWidget = contentStack->widget(currentIndex);
            contentStack->removeWidget(oldWidget);
            oldWidget->deleteLater();
            contentStack->insertWidget(currentIndex, newWidget);
            contentStack->setCurrentIndex(currentIndex);
        }
    }

    // Update the sidebar theme
    if (leftSidebar) {
        leftSidebar->updateTheme(isDark);
    }

    update();
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
    qDebug() << "Setting up UI";
    
    //main horizontal layout (leftsidebar + contentStack)
    mainHLayout = new QHBoxLayout(this);
    mainHLayout->setContentsMargins(0, 0, 0, 0);
    mainHLayout->setSpacing(0);

    //create and define sidebar buttons (also it's initial theme)
    leftSidebar = new LeftSidebar();
    leftSidebar->addButton(":/Images/whistle.png", "Gym Classes", "classes");
    leftSidebar->addButton(":/Images/muscle.png", "Workouts", "workouts");
    leftSidebar->addButton(":/Images/team.png", "Add Class", "add-classes");
    leftSidebar->updateTheme(true);
    mainHLayout->addWidget(leftSidebar);
    
    // Create content stack and add pages in correct order
    contentStack = new QStackedWidget();
    
    // Create and add pages in specific order
    QWidget* classesPage = ClassesContent();
    QWidget* workoutsPage = WorkoutsContent();
    QWidget* historyPage = ExtraContent();
    
    contentStack->addWidget(classesPage);    // Index 0 - Classes
    contentStack->addWidget(workoutsPage);   // Index 1 - Workouts
    contentStack->addWidget(historyPage);    // Index 2 - History
    
    mainHLayout->addWidget(contentStack);

    // Important: Set the current index AFTER all pages are added
    contentStack->setCurrentIndex(0);  // Explicitly set to classes page
    leftSidebar->setActiveButton("classes");

    // Connect signal AFTER setting initial state to avoid triggering during setup
    QObject::disconnect(leftSidebar, &LeftSidebar::pageChanged, this, &AvailableClassesScreen::handlePageChange);
    QObject::connect(leftSidebar, &LeftSidebar::pageChanged, this, &AvailableClassesScreen::handlePageChange);
    std::cout << "Signal connections established" << std::endl;

    setLayout(mainHLayout);
    qDebug() << "UI Setup complete - Current page index:" << contentStack->currentIndex();
    std::cout << "Current widget class name: " << contentStack->currentWidget()->metaObject()->className() << std::endl;

    // Force an immediate update of the current widget
    if (contentStack->currentWidget()) {
        contentStack->currentWidget()->update();
        std::cout << "Current widget updated" << std::endl;
    }

    std::cout << "=== setupUI Complete ===\n" << std::endl;
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

    int memberId = -1;
    if (memberDataManager) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
    }

    if (memberId > 0 && currentMember.getClassId() > 0) {
        Class enrolledClass = classDataManager->getClassById(currentMember.getClassId());
        if (enrolledClass.getId() > 0) {
            names << enrolledClass.getClassName();
            qDebug() << "Found enrolled class:" << enrolledClass.getClassName() << "with ID:" << enrolledClass.getId();
        }
    }

    if (memberId > 0) {
        for (const auto& gymClass : all) {
            if (gymClass.isMemberEnrolled(memberId) && !names.contains(gymClass.getClassName())) {
                names << gymClass.getClassName();
                qDebug() << "Found additional enrollment in class:" << gymClass.getClassName() << "with ID:" << gymClass.getId();
            }
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
        background-color: rgba(139, 92, 246, 0.05);
        border-radius: 12px;
        padding: 16px;
        border: 1px solid #BFAEF5;
    }
    QLabel {
        font-size: 14px;
        color: #978ADD;
    }
    QLabel#title {
        font-size: 18px;
        font-weight: bold;
        color: #4B3C9C;
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

    int memberId = -1;
    if (memberDataManager && currentUser.getId() > 0) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
    }

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

    int memberId = -1;
    bool isVIP = false;

    if (memberDataManager && currentUser.getId() > 0) {
        memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
        if (memberId > 0) {
            isVIP = memberDataManager->isVIPMember(memberId);
        }
    }

    if (memberId <= 0) {
        NotificationManager::instance().showNotification(
            tr("Waitlist Failed"),
            "You must be a member to join the waitlist",
            nullptr,
            NotificationType::Error
        );
        return;
    }

    if (!classDataManager->addToWaitlist(classId, memberId, isVIP, error)) {
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
    fromDateEdit->setDate(timeLogicInstance.getCurrentTime().date());
    fromDateEdit->setDisplayFormat("dd/MM/yyyy");

    QDateEdit *toDateEdit = new QDateEdit;
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDate(timeLogicInstance.getCurrentTime().date().addDays(7));
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
        newClass.setCoachName(coachComboBox->currentText());
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
    qDebug() << "Creating ClassesContent";
    
    QWidget* classesContent = new QWidget();
    QVBoxLayout* classesLayout = new QVBoxLayout(classesContent);
    classesLayout->setContentsMargins(20,20,20,20);
    classesLayout->setSpacing(15);

    userNameLabel = new QLabel(currentUser.getId() > 0
        ? QString("Hello, %1!").arg(currentUser.getName())
        : "Hello, Guest!");
    userNameLabel->setStyleSheet("font-size:20px; font-weight:bold;");
    
    enrolledClassesLabel = new QLabel;
    enrolledClassesLabel->setWordWrap(true);
    enrolledClassesLabel->setStyleSheet("color:#555;");

    if (currentUser.getId() > 0) {
        auto all = classDataManager->getAllClasses();
        QStringList names;

        int memberId = -1;
        if (memberDataManager) {
            memberId = memberDataManager->getMemberIdByUserId(currentUser.getId());
        }

        if (memberId > 0 && currentMember.getClassId() > 0) {
            Class enrolledClass = classDataManager->getClassById(currentMember.getClassId());
            if (enrolledClass.getId() > 0) {
                names << enrolledClass.getClassName();
            }
        }

        if (memberId > 0) {
            for (const auto& gymClass : all) {
                if (gymClass.isMemberEnrolled(memberId) && !names.contains(gymClass.getClassName())) {
                    names << gymClass.getClassName();
                }
            }
        }

        if (names.isEmpty()) {
            enrolledClassesLabel->setText("You're not enrolled in any classes yet.");
        } else {
            enrolledClassesLabel->setText(QString("Your classes: %1").arg(names.join(", ")));
        }
    } else {
        enrolledClassesLabel->setText("Please log in to view your classes.");
    }

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
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setSpacing(30);

    auto all = classDataManager->getAllClasses();
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
    classesLayout->addWidget(scrollArea);

    qDebug() << "ClassesContent created successfully";
    return classesContent;
}
QWidget* AvailableClassesScreen::WorkoutsContent() {
    qDebug() << "Starting WorkoutsContent() function";

    QWidget* workoutsContent = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(workoutsContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel* titleLabel = new QLabel("Monthly Workout Schedule");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #6647D8;");
    mainLayout->addWidget(titleLabel);

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

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(15);

    if (!workoutManager) {
        qDebug() << "ERROR: workoutManager is null!";
        QLabel* errorLabel = new QLabel("Workout system is currently unavailable.");
        errorLabel->setStyleSheet("color: red;");
        mainLayout->addWidget(errorLabel);
        return workoutsContent;
    }

    QVector<Workout> allWorkouts = workoutManager->getAllWorkouts();
    if (allWorkouts.isEmpty()) {
        qDebug() << "ERROR: No workouts found in the system!";
        QLabel* errorLabel = new QLabel("No workouts available.");
        errorLabel->setStyleSheet("color: red;");
        mainLayout->addWidget(errorLabel);
        return workoutsContent;
    }

    qDebug() << "Found" << allWorkouts.size() << "workouts in the system";

    QDate today = timeLogicInstance.getCurrentTime().date();
    QDate endOfMonth = QDate(today.year(), today.month(), today.daysInMonth());

    for (QDate date = today; date <= endOfMonth; date = date.addDays(1)) {
        qDebug() << "Processing date:" << date.toString();

        bool skipDate = false;

        if (currentUser.getId() > 0) {
            QVector<WorkoutLog> logs = workoutManager->getUserWorkoutLogsByDateRange(
                currentUser.getId(), date, date);
            if (!logs.isEmpty()) {
                qDebug() << "Workout already completed for" << date.toString();
                skipDate = true;
            }
        }

        if (skipDate) {
            continue;
        }

        int randomIndex = QRandomGenerator::global()->bounded(allWorkouts.size());
        const Workout& workout = allWorkouts[randomIndex];
        qDebug() << "Selected workout:" << workout.name << "for date:" << date.toString();

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

        QLabel* dateLabel = new QLabel(date.toString("dddd, MMMM d"));
        dateLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #6647D8;");
        cardLayout->addWidget(dateLabel);

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
        connect(trainButton, &QPushButton::clicked, [this, workout, card, date, today]() {
            qDebug() << "Logging workout:" << workout.name << "for date:" << date.toString();
            
            if (date > today) {
                QMessageBox::warning(this, "Invalid Date", "You can't train at a future date.");
                return;
            }

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

            for (const Exercise& exercise : workout.exercises) {
                log.completedExercises.append(qMakePair(exercise.name, true));
            }

            QString errorMessage;
            if (workoutManager->logWorkout(log, errorMessage)) {
                QMessageBox::information(this, "Success", 
                    QString("Completed workout: %1\nCalories burned: %2")
                    .arg(workout.name)
                    .arg(workout.totalCalories));

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

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    qDebug() << "WorkoutsContent() function completed";
    return workoutsContent;
}
QWidget* AvailableClassesScreen::ExtraContent() {
    qDebug() << "Starting ExtraContent() function";

    QWidget* historyContent = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(historyContent);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Set background based on theme
    QString bgColor = isDarkTheme ? "#1A1A2E" : "#FAF5FF";
    QString textColor = isDarkTheme ? "#E2E8F0" : "#1E293B";
    QString mutedTextColor = isDarkTheme ? "#8B8B9E" : "#64748B";
    QString accentColor = isDarkTheme ? "#A78CF6" : "#6647D8";
    QString cardBgColor = isDarkTheme ? "rgba(167, 140, 246, 0.05)" : "#FFFFFF";
    QString cardBorderColor = isDarkTheme ? "#2D2D44" : "#E2E8F0";

    historyContent->setStyleSheet(QString("background-color: %1;").arg(bgColor));

    // Title section
    QWidget* titleWidget = new QWidget();
    QVBoxLayout* titleLayout = new QVBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 20);
    titleLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel("Workout History");
    titleLabel->setStyleSheet(QString("font-size: 32px; font-weight: bold; color: %1;").arg(accentColor));
    titleLayout->addWidget(titleLabel);

    QLabel* subtitleLabel = new QLabel("Track your fitness journey and progress");
    subtitleLabel->setStyleSheet(QString("font-size: 16px; color: %1;").arg(mutedTextColor));
    titleLayout->addWidget(subtitleLabel);
    mainLayout->addWidget(titleWidget);

    // Validations with theme-aware styling
    if (!workoutManager) {
        QLabel* errorLabel = new QLabel("Workout system is currently unavailable.");
        errorLabel->setStyleSheet("color: #FF6B6B; font-size: 16px; padding: 20px;");
        mainLayout->addWidget(errorLabel);
        return historyContent;
    }
    if (currentUser.getId() <= 0) {
        QWidget* loginPrompt = new QWidget();
        loginPrompt->setStyleSheet(QString(
            "background-color: %1;"
            "border: 1px solid %2;"
            "border-radius: 12px;"
            "padding: 20px;"
        ).arg(cardBgColor, cardBorderColor));
        QVBoxLayout* promptLayout = new QVBoxLayout(loginPrompt);
        QLabel* loginLabel = new QLabel("Please log in to view your workout history");
        loginLabel->setStyleSheet(QString("color: %1; font-size: 16px;").arg(textColor));
        promptLayout->addWidget(loginLabel);
        mainLayout->addWidget(loginPrompt);
        return historyContent;
    }

    // Fetch logs
    QVector<WorkoutLog> userLogs = workoutManager->getUserWorkoutLogs(currentUser.getId());

    // Optimized scroll area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QString(
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:vertical {"
        "   background: %1;"
        "   width: 8px;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: %2;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
    ).arg(cardBgColor, cardBorderColor));

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(20);
    scrollLayout->setContentsMargins(0, 0, 8, 0);

    if (userLogs.isEmpty()) {
        QWidget* emptyState = new QWidget();
        emptyState->setStyleSheet(QString(
            "background-color: %1;"
            "border: 1px solid %2;"
            "border-radius: 12px;"
            "padding: 30px;"
        ).arg(cardBgColor, cardBorderColor));
        QVBoxLayout* emptyLayout = new QVBoxLayout(emptyState);
        QLabel* emptyLabel = new QLabel("No workout history found yet");
        emptyLabel->setStyleSheet(QString("font-size: 18px; color: %1; font-weight: bold;").arg(textColor));
        QLabel* emptySubLabel = new QLabel("Start training to begin tracking your fitness journey!");
        emptySubLabel->setStyleSheet(QString("font-size: 14px; color: %1; margin-top: 8px;").arg(mutedTextColor));
        emptyLayout->addWidget(emptyLabel, 0, Qt::AlignCenter);
        emptyLayout->addWidget(emptySubLabel, 0, Qt::AlignCenter);
        mainLayout->addWidget(emptyState);
    } else {
        // Stats cards
        QWidget* statsContainer = new QWidget();
        QHBoxLayout* statsLayout = new QHBoxLayout(statsContainer);
        statsLayout->setSpacing(20);
        
        // Total Workouts Card
        QWidget* workoutsCard = new QWidget();
        workoutsCard->setStyleSheet(QString(
            "background: %1;"
            "border: 1px solid %2;"
            "border-radius: 12px;"
            "padding: 20px;"
        ).arg(cardBgColor, cardBorderColor));
        QVBoxLayout* workoutsLayout = new QVBoxLayout(workoutsCard);
        workoutsLayout->setSpacing(4);

        int totalWorkouts = workoutManager->getTotalWorkoutsCompleted(currentUser.getId());
        QLabel* workoutCount = new QLabel(QString::number(totalWorkouts));
        workoutCount->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(accentColor));
        QLabel* workoutLabel = new QLabel("Total Workouts");
        workoutLabel->setStyleSheet(QString("font-size: 14px; color: %1;").arg(mutedTextColor));
        workoutsLayout->addWidget(workoutCount);
        workoutsLayout->addWidget(workoutLabel);
        statsLayout->addWidget(workoutsCard);
        
        // Total Calories Card
        QWidget* caloriesCard = new QWidget();
        caloriesCard->setStyleSheet(QString(
            "background: %1;"
            "border: 1px solid %2;"
            "border-radius: 12px;"
            "padding: 20px;"
        ).arg(cardBgColor, cardBorderColor));
        QVBoxLayout* caloriesLayout = new QVBoxLayout(caloriesCard);
        caloriesLayout->setSpacing(4);

        int totalCalories = workoutManager->getTotalCaloriesBurnt(currentUser.getId());
        QLabel* caloriesCount = new QLabel(QString::number(totalCalories));
        caloriesCount->setStyleSheet(QString("font-size: 28px; font-weight: bold; color: %1;").arg(accentColor));
        QLabel* caloriesLabel = new QLabel("Total Calories Burned");
        caloriesLabel->setStyleSheet(QString("font-size: 14px; color: %1;").arg(mutedTextColor));
        caloriesLayout->addWidget(caloriesCount);
        caloriesLayout->addWidget(caloriesLabel);
        statsLayout->addWidget(caloriesCard);
        
        scrollLayout->addWidget(statsContainer);

        // Grid layout for 3-column cards
        QGridLayout* cardsGrid = new QGridLayout();
        cardsGrid->setSpacing(20);
        int row = 0, col = 0;

        // Workout History Cards
        for (const WorkoutLog& log : userLogs) {
            QWidget* card = new QWidget();
            card->setFixedWidth(300);

            // Add shadow effect
            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
            shadow->setBlurRadius(15);
            shadow->setXOffset(0);
            shadow->setYOffset(4);
            shadow->setColor(QColor(0, 0, 0, 40));
            card->setGraphicsEffect(shadow);

            card->setStyleSheet(QString(
                "background: %1;"
                "border: 1px solid %2;"
                "border-radius: 12px;"
                "padding: 16px;"
            ).arg(cardBgColor, cardBorderColor));

            QVBoxLayout* cardLayout = new QVBoxLayout(card);
            cardLayout->setSpacing(12);

            Workout workout = workoutManager->getWorkoutById(log.workoutId);

            // Workout name
            QLabel* workoutName = new QLabel(workout.name);
            workoutName->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(textColor));
            workoutName->setWordWrap(true);
            cardLayout->addWidget(workoutName);
            
            // Date and calories
            QWidget* infoWidget = new QWidget();
            QHBoxLayout* infoLayout = new QHBoxLayout(infoWidget);
            infoLayout->setContentsMargins(0, 0, 0, 0);

            QLabel* workoutDate = new QLabel(log.timestamp.toString("MMM d, h:mm ap"));
            workoutDate->setStyleSheet(QString("font-size: 12px; color: %1;").arg(mutedTextColor));
            infoLayout->addWidget(workoutDate);

            QLabel* calories = new QLabel(QString("%1 cal").arg(log.totalCaloriesBurnt));
            calories->setStyleSheet(QString(
                "color: %1;"
                "background: %2;"
                "border-radius: 10px;"
                "padding: 4px 8px;"
                "font-size: 12px;"
            ).arg(accentColor, cardBgColor));
            infoLayout->addWidget(calories);
            cardLayout->addWidget(infoWidget);

            // Divider
            QFrame* divider = new QFrame();
            divider->setFrameShape(QFrame::HLine);
            divider->setStyleSheet(QString("background-color: %1;").arg(cardBorderColor));
            cardLayout->addWidget(divider);

            // Exercises
            QWidget* exercisesWidget = new QWidget();
            QVBoxLayout* exercisesLayout = new QVBoxLayout(exercisesWidget);
            exercisesLayout->setSpacing(8);
            exercisesLayout->setContentsMargins(0, 0, 0, 0);

            int displayCount = 0;
            for (const auto& exercise : log.completedExercises) {
                if (displayCount >= 3) break;

                QWidget* exerciseItem = new QWidget();
                QHBoxLayout* exerciseLayout = new QHBoxLayout(exerciseItem);
                exerciseLayout->setContentsMargins(0, 0, 0, 0);

                QString statusColor = exercise.second ? "#10B981" : "#EF4444";
                QString statusBg = exercise.second ?
                    (isDarkTheme ? "rgba(16, 185, 129, 0.2)" : "rgba(16, 185, 129, 0.1)") :
                    (isDarkTheme ? "rgba(239, 68, 68, 0.2)" : "rgba(239, 68, 68, 0.1)");

                QLabel* status = new QLabel(exercise.second ? "✓" : "×");
                status->setStyleSheet(QString(
                    "color: %1;"
                    "background: %2;"
                    "border-radius: 8px;"
                    "padding: 2px 6px;"
                    "font-size: 12px;"
                ).arg(statusColor, statusBg));
                exerciseLayout->addWidget(status);

                QLabel* exerciseName = new QLabel(exercise.first);
                exerciseName->setStyleSheet(QString("font-size: 12px; color: %1;").arg(mutedTextColor));
                exerciseName->setWordWrap(true);
                exerciseLayout->addWidget(exerciseName);

                exercisesLayout->addWidget(exerciseItem);
                displayCount++;
            }

            if (log.completedExercises.size() > 3) {
                QLabel* moreExercises = new QLabel(
                    QString("+ %1 more").arg(log.completedExercises.size() - 3)
                );
                moreExercises->setStyleSheet(QString(
                    "color: %1;"
                    "font-size: 12px;"
                    "margin-top: 4px;"
                ).arg(mutedTextColor));
                moreExercises->setAlignment(Qt::AlignRight);
                exercisesLayout->addWidget(moreExercises);
            }

            cardLayout->addWidget(exercisesWidget);
            cardsGrid->addWidget(card, row, col);

            if (++col >= 3) {
                col = 0;
                row++;
            }
        }

        scrollLayout->addLayout(cardsGrid);
    }

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    return historyContent;
}
void AvailableClassesScreen::handlePageChange(const QString& pageID) {
    std::cout << "\n=== Page Change Requested ===" << std::endl;
    qDebug() << "Current page index:" << contentStack->currentIndex();
    qDebug() << "Requested page:" << pageID;
    qDebug() << "Current workoutManager:" << workoutManager;

    // Map page IDs to indices
    int targetIndex = 0; // Default to Classes

    if (pageID == "classes") {
        targetIndex = 0;
        std::cout << "Switching to Classes page (index 0)" << std::endl;
    } else if (pageID == "workouts") {
        targetIndex = 1;
        std::cout << "Switching to Workouts page (index 1)" << std::endl;
    } else if (pageID == "add-classes") {
        targetIndex = 2;
        std::cout << "Switching to History page (index 2)" << std::endl;
    }

    std::cout << "Current index before change: " << contentStack->currentIndex() << std::endl;
    std::cout << "Target index: " << targetIndex << std::endl;

    // First, recreate the target page to ensure it's up-to-date
    QWidget* newWidget = nullptr;

    std::cout << "Creating new widget for target page" << std::endl;
    switch (targetIndex) {
        case 0:
            std::cout << "Creating Classes content" << std::endl;
            newWidget = ClassesContent();
            break;
        case 1:
            std::cout << "Creating Workouts content" << std::endl;
            newWidget = WorkoutsContent();
            break;
        case 2:
            std::cout << "Creating History content" << std::endl;
            newWidget = ExtraContent();
            break;
    }

    if (newWidget) {
        std::cout << "Replacing widget at index " << targetIndex << std::endl;
        // Replace the widget at the target index
        QWidget* oldWidget = contentStack->widget(targetIndex);
        contentStack->removeWidget(oldWidget);
        oldWidget->deleteLater();
        contentStack->insertWidget(targetIndex, newWidget);
    }

    // Set to the target index
    std::cout << "Setting current index to " << targetIndex << std::endl;
    contentStack->setCurrentIndex(targetIndex);

    // Force updates
    std::cout << "Refreshing classes" << std::endl;
    refreshClasses();
    update();
}
