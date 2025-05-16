#include "BookingWindow.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QTableWidgetItem>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QScrollArea>

#include "Stylesheets/Padel/BookingWindowStyle.h"

BookingWindow::BookingWindow(PadelDataManager* padelManager, QWidget* parent)
    : QWidget(parent), m_padelManager(padelManager), m_currentUserId(0), m_isDarkTheme(false)
{
    setupUI();
    setupConnections();
    loadCourts();
    
    QTimer::singleShot(100, this, &BookingWindow::updateAlternativeCourts);
}

void BookingWindow::setCurrentUserEmail(const QString& email)
{
    m_currentUserEmail = email;
    
    m_currentUserId = 0;
    m_userInfoLabel->setText(tr("User: %1").arg(email));
    
    if (m_userDataManager) {
        User user = m_userDataManager->getUserData(email);
        if (user.getId() > 0) {
            m_currentUserId = user.getId();
            qDebug() << "User ID set to" << m_currentUserId << "for email" << email;

            bool isMember = false;
            if (m_memberDataManager) {
                isMember = m_memberDataManager->userIsMember(m_currentUserId);
                if (isMember) {
                    int memberId = m_memberDataManager->getMemberIdByUserId(m_currentUserId);
                    qDebug() << "User" << m_currentUserId << "is a member with ID:" << memberId;
                    
                    m_userInfoLabel->setText(tr("User: %1 (Member ID: %2)").arg(email).arg(memberId));
                } else {
                    m_userInfoLabel->setText(tr("User: %1 (Not a member)").arg(email));
                }
            }
        } else {
            qDebug() << "No user found for email" << email;
        }
    }

    qDebug() << "User changed - clearing and refreshing all booking data";
    clearTimeSlotGrid();
    if (m_courtSelector && m_courtSelector->count() > 0) {
        int courtId = m_courtSelector->currentData().toInt();
        if (courtId > 0) {
            //updateCourtDetails(courtId);
            refreshTimeSlots();
        }
    }
    
    refreshBookingsList();
}

void BookingWindow::loadUserData()
{
    if (!m_userDataManager) {
        qDebug() << "Error: UserDataManager is not set";
        return;
    }

    if (m_currentUserEmail.isEmpty()) {
        qDebug() << "Error: Current user email is empty";
        return;
    }

    User user = m_userDataManager->getUserData(m_currentUserEmail);
    if (user.getId() <= 0) {
        qDebug() << "Error: Invalid user ID for email" << m_currentUserEmail;
        return;
    }

    m_currentUserId = user.getId();
    qDebug() << "Loaded user ID" << m_currentUserId << "from email" << m_currentUserEmail;

    bool isMember = false;
    int memberId = -1;
    
    if (m_memberDataManager) {
        isMember = m_memberDataManager->userIsMember(m_currentUserId);
        if (isMember) {
            memberId = m_memberDataManager->getMemberIdByUserId(m_currentUserId);
            qDebug() << "User is a member with member ID:" << memberId;
        } else {
            qDebug() << "User is not a member";
        }
    }

    QString userInfo = tr("User: %1").arg(m_currentUserEmail);
    if (isMember) {
        userInfo += tr(" (Member ID: %1)").arg(memberId);
    } else {
        userInfo += tr(" (Not a member)");
    }
    
    if (m_userInfoLabel) {
        m_userInfoLabel->setText(userInfo);
    }
}

void BookingWindow::setupUI()
{
    QHBoxLayout* rootLayout = new QHBoxLayout(this);
    rootLayout->setSpacing(0);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    m_leftSidebar = new LeftSidebar(this);
    m_leftSidebar->addButton(":/Images/booking.png", tr("Booking"), "booking-section");
    m_leftSidebar->addButton(":/Images/alternative.png", tr("Alternative"), "alternative-section");
    m_leftSidebar->addButton(":/Images/my_bookings.png", tr("My Bookings"), "mybookings-section");
    connect(m_leftSidebar, &LeftSidebar::pageChanged, this, &BookingWindow::handleSidebarPageChange);
    m_contentStack = new QStackedWidget();
    m_contentStack->setStyleSheet("QStackedWidget { background: transparent; }");

    QScrollArea* scroll1 = new QScrollArea();
    scroll1->setWidgetResizable(true);
    scroll1->setFrameShape(QFrame::NoFrame);
    scroll1->setStyleSheet(R"(
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
    )");
    m_section1 = new QWidget();
    m_section1->setStyleSheet("background: transparent;");
    scroll1->setWidget(m_section1);

    QScrollArea* scroll2 = new QScrollArea();
    scroll2->setWidgetResizable(true);
    scroll2->setFrameShape(QFrame::NoFrame);
    scroll2->setStyleSheet(scroll1->styleSheet());
    m_section2 = new QWidget();
    m_section2->setStyleSheet("background: transparent;");
    scroll2->setWidget(m_section2);

    QScrollArea* scroll3 = new QScrollArea();
    scroll3->setWidgetResizable(true);
    scroll3->setFrameShape(QFrame::NoFrame);
    scroll3->setStyleSheet(scroll1->styleSheet());
    m_section3 = new QWidget();
    m_section3->setStyleSheet("background: transparent;");
    scroll3->setWidget(m_section3);

    QVBoxLayout* section1Layout = new QVBoxLayout(m_section1);
    section1Layout->setContentsMargins(30, 30, 30, 30);
    section1Layout->setSpacing(18);

    QLabel* titleLabel = new QLabel(tr("Padel Court Booking System"), m_section1);
    titleLabel->setStyleSheet(titleLabelStyle);
    titleLabel->setObjectName("titleLabel");
    section1Layout->addWidget(titleLabel);
    m_userInfoLabel = new QLabel(tr("User: Not logged in"), m_section1);
    m_userInfoLabel->setStyleSheet(userInfoLabelStyle);
    m_userInfoLabel->setObjectName("userInfoLabel");
    section1Layout->addWidget(m_userInfoLabel);
    section1Layout->addWidget(createSeparator());
    setupSearchUI(m_section1, section1Layout);
    section1Layout->addWidget(createSeparator());

    QHBoxLayout* courtSelectionLayout = new QHBoxLayout();
    section1Layout->addLayout(courtSelectionLayout);
    QVBoxLayout* selectionLayout = new QVBoxLayout();
    selectionLayout->setSpacing(12);
    courtSelectionLayout->addLayout(selectionLayout, 3);
    QHBoxLayout* courtLayout = new QHBoxLayout();
    QLabel* courtLabel = new QLabel(tr("Select Court:"), m_section1);
    courtLabel->setStyleSheet(courtLabelStyle);
    courtLabel->setObjectName("labelHeading");
    m_courtSelector = new QComboBox(m_section1);
    m_courtSelector->setMinimumWidth(200);
    m_courtSelector->setStyleSheet(courtSelectorStyle);
    courtLayout->addWidget(courtLabel);
    courtLayout->addWidget(m_courtSelector);
    selectionLayout->addLayout(courtLayout);
    QHBoxLayout* dateLayout = new QHBoxLayout();
    QLabel* dateLabel = new QLabel(tr("Select Date:"), m_section1);
    dateLabel->setStyleSheet(dateLabelStyle);
    dateLabel->setObjectName("labelHeading");   
    m_dateSelector = new QDateEdit(timeLogicInstance.getCurrentTime().date(), m_section1);
    m_dateSelector->setCalendarPopup(true);
    m_dateSelector->setMinimumDate(timeLogicInstance.getCurrentTime().date());
    m_dateSelector->setStyleSheet(dateSelectorStyle);
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(m_dateSelector);
    selectionLayout->addLayout(dateLayout);

    // ============================= WEEK NAVIGATION =============================
    QHBoxLayout* weekNavLayout = new QHBoxLayout();
    weekNavLayout->setAlignment(Qt::AlignCenter);
    m_prevWeekButton = new QPushButton("<---", m_section1);
    m_nextWeekButton = new QPushButton("--->", m_section1);
    m_weekRangeLabel = new QLabel(m_section1);
    m_prevWeekButton->setStyleSheet(weekButtonStyle);
    m_prevWeekButton->setCursor(Qt::PointingHandCursor);
    m_nextWeekButton->setCursor(Qt::PointingHandCursor);
    m_nextWeekButton->setStyleSheet(weekButtonStyle);
    m_weekRangeLabel->setStyleSheet(weekLabelStyle);
    weekNavLayout->addWidget(m_prevWeekButton);
    weekNavLayout->addWidget(m_weekRangeLabel);
    weekNavLayout->addWidget(m_nextWeekButton);
    selectionLayout->addLayout(weekNavLayout);
    connect(m_prevWeekButton, &QPushButton::clicked, this, &BookingWindow::showPreviousWeek);
    connect(m_nextWeekButton, &QPushButton::clicked, this, &BookingWindow::showNextWeek);
    // =========================== END WEEK NAVIGATION ===========================

    QLabel* timeSlotsTitle = new QLabel(tr("Available Time Slots:"), m_section1);
    timeSlotsTitle->setStyleSheet(timeSlotsTitleStyle);
    timeSlotsTitle->setObjectName("sectionHeading");
    selectionLayout->addWidget(timeSlotsTitle);


    //================================= Old grid ==================================
    //
    // m_slotsLayout = new QGridLayout();
    // m_slotsLayout->setSpacing(10);
    // m_slotsLayout->setContentsMargins(5, 5, 5, 5);
    //
    // QWidget* slotsContainer = new QWidget(calendarContainer);
    // slotsContainer->setLayout(m_slotsLayout);
    // m_calendarGrid->addWidget(slotsContainer, 0, 0);
    //
    // m_noSlotsLabel = new QLabel(tr("No time slots available"), calendarContainer);
    // m_noSlotsLabel->setAlignment(Qt::AlignCenter);
    // m_noSlotsLabel->setStyleSheet("color: #6B7280; font-style: italic;");
    // m_noSlotsLabel->setObjectName("messageLabel");
    // m_noSlotsLabel->hide();
    // m_calendarGrid->addWidget(m_noSlotsLabel, 1, 0);
    //================================= New grid ==================================
    QWidget* calendarContainer = new QWidget(m_section1);
    calendarContainer->setMinimumHeight(600);
    selectionLayout->addWidget(calendarContainer);
    m_calendarGrid = new QGridLayout(calendarContainer);
    m_calendarGrid->setSpacing(2);
    m_calendarGrid->setContentsMargins(5, 5, 5, 5);

    // Add corner header (top-left cell)
    QLabel* cornerLabel = new QLabel("Day\\Time", calendarContainer);
    cornerLabel->setAlignment(Qt::AlignCenter);
    cornerLabel->setStyleSheet(calendarCornerLabelStyle.arg(m_isDarkTheme ? "black" : "white"));
    m_calendarGrid->addWidget(cornerLabel, 0, 0);

    // Add time headers (column headers, row 0)
    for (int col = 0; col < 15; col++) {
        QTime time = QTime(7, 0).addSecs(col * 3600); // Starting at 7:00, 1-hour increments
        QTime endTime = time.addSecs(3600);  // Add 1 hour
        QString intervalText = QString("%1\nTo\n%2").arg(
            time.toString("h:mm AP"),
            endTime.toString("h:mm AP")
        );
        QLabel* timeLabel = new QLabel(intervalText, calendarContainer);
        timeLabel->setAlignment(Qt::AlignCenter);
        timeLabel->setStyleSheet(calendarTimeLabelStyle.arg(m_isDarkTheme ? "#9CAFF0" : "#33498F", m_isDarkTheme ? "black" : "white"));
        m_calendarGrid->addWidget(timeLabel, 0, col + 1); // +1 to leave room for day headers
    }

    // Add day-of-week headers (row headers, column 0)
    QStringList days = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    for (int row = 0; row < 7; row++) {
        QLabel* dayLabel = new QLabel(days[row], calendarContainer);
        dayLabel->setAlignment(Qt::AlignCenter);
        dayLabel->setStyleSheet(calendarDayLabelStyle.arg(m_isDarkTheme ? "#9CAFF0" : "#33498F", m_isDarkTheme ? "black" : "white"));
        m_dayHeaders[row] = dayLabel;
        m_calendarGrid->addWidget(dayLabel, row + 1, 0); // +1 to leave room for time headers
    }

    // Initialize calendar cells (7 days × 15 time slots)
    for (int day = 0; day < 7; day++) {
        for (int timeSlot = 0; timeSlot < 15; timeSlot++) {
            CalendarButton* cellButton = new CalendarButton(this);
            cellButton->setMinimumSize(90, 70); // Adjust size as needed

            m_calendarGrid->addWidget(cellButton, day + 1,timeSlot + 1); // +1 to account for headers
            m_calendarCells[day][timeSlot] = cellButton; // Store in a 2D array for easy access
        }
    }

    //=============================================================================

    QVBoxLayout* section2Layout = new QVBoxLayout(m_section2);
    section2Layout->setContentsMargins(30, 30, 30, 30);
    section2Layout->setSpacing(18);

    QLabel* alternativeTitle = new QLabel(tr("Alternative Courts"), m_section2);
    alternativeTitle->setStyleSheet(titleLabelStyle);
    alternativeTitle->setObjectName("titleLabel");
    section2Layout->addWidget(alternativeTitle);

    QWidget* alternativeCourtsWidget = setupAlternativeCourtsUI(m_section2);
    section2Layout->addWidget(alternativeCourtsWidget);
    QHBoxLayout* timeLayout = new QHBoxLayout();
    QLabel* timeLabel = new QLabel(tr("Select Time:"), m_section2);
    m_timeSlotSelector = new QComboBox(m_section2);
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(m_timeSlotSelector);

    timeLabel->setVisible(false);
    m_timeSlotSelector->setVisible(false);
    section2Layout->addLayout(timeLayout);
    QLayout* waitlistLayout = setupWaitlistUI(m_section2);
    section2Layout->addLayout(waitlistLayout);
    m_statusLabel = new QLabel(m_section2);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(statusLabelStyle);
    m_statusLabel->setObjectName("statusLabel");
    section2Layout->addWidget(m_statusLabel);
    section2Layout->addStretch();

    // QVBoxLayout* detailsLayout = new QVBoxLayout();
    // courtSelectionLayout->addLayout(detailsLayout, 2);
    //
    // QLabel* detailsTitle = new QLabel(tr("Court Details"), contentWidget);
    // detailsTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #4F46E5;");
    // detailsTitle->setObjectName("sectionHeading");
    // detailsLayout->addWidget(detailsTitle);
    //
    // m_courtNameLabel = new QLabel(contentWidget);
    // m_courtLocationLabel = new QLabel(contentWidget);
    // m_courtPriceLabel = new QLabel(contentWidget);
    // m_courtDescriptionLabel = new QLabel(contentWidget);
    // m_courtDescriptionLabel->setWordWrap(true);
    //
    // QString labelStyle = "padding: 5px; margin-bottom: 5px; color: #111827;";
    // m_courtNameLabel->setStyleSheet(labelStyle);
    // m_courtLocationLabel->setStyleSheet(labelStyle);
    // m_courtPriceLabel->setStyleSheet(labelStyle);
    // m_courtDescriptionLabel->setStyleSheet(labelStyle);
    //
    // m_courtNameLabel->setObjectName("detailLabel");
    // m_courtLocationLabel->setObjectName("detailLabel");
    // m_courtPriceLabel->setObjectName("detailLabel");
    // m_courtDescriptionLabel->setObjectName("detailLabel");
    //
    // detailsLayout->addWidget(m_courtNameLabel);
    // detailsLayout->addWidget(m_courtLocationLabel);
    // detailsLayout->addWidget(m_courtPriceLabel);
    //
    // m_capacityLabel = new QLabel(contentWidget);
    // m_capacityLabel->setObjectName("capacityLabel");
    // detailsLayout->addWidget(m_capacityLabel);
    //
    // detailsLayout->addWidget(createSeparator());
    //
    // detailsLayout->addWidget(m_courtDescriptionLabel);
    //
    // QLabel* featuresTitle = new QLabel(tr("Features:"), contentWidget);
    // featuresTitle->setStyleSheet("font-weight: bold; margin-top: 10px; color: #4F46E5;");
    // featuresTitle->setObjectName("subHeading");
    // detailsLayout->addWidget(featuresTitle);
    //
    // m_courtFeaturesListWidget = new QListWidget(contentWidget);
    // m_courtFeaturesListWidget->setMaximumHeight(120);
    // m_courtFeaturesListWidget->setStyleSheet("background-color: transparent; border: none;");
    // detailsLayout->addWidget(m_courtFeaturesListWidget);
    //
    // detailsLayout->addStretch();

    QVBoxLayout* section3Layout = new QVBoxLayout(m_section3);
    section3Layout->setContentsMargins(30, 30, 30, 30);
    section3Layout->setSpacing(18);

    QLabel* bookingsTitle = new QLabel(tr("My Bookings"), m_section3);
    bookingsTitle->setStyleSheet(titleLabelStyle);
    bookingsTitle->setObjectName("titleLabel");
    section3Layout->addWidget(bookingsTitle);

    QWidget* bookingsContainer = new QWidget(m_section3);
    bookingsContainer->setObjectName("bookingsContainer");
    bookingsContainer->setStyleSheet(QString(R"(
        QWidget#bookingsContainer {
            background: rgba(%1, 0.6);
            border-radius: 16px;
            border: 1px solid rgba(139, 92, 246, 0.2);
        }
    )").arg(m_isDarkTheme ? "0, 0, 0" : "255, 255, 255"));

    QVBoxLayout* bookingsContainerLayout = new QVBoxLayout(bookingsContainer);
    bookingsContainerLayout->setContentsMargins(20, 20, 20, 20);
    bookingsContainerLayout->setSpacing(20);
    m_bookingsList = new QListWidget(bookingsContainer);
    m_bookingsList->setMinimumHeight(200);
    m_bookingsList->setStyleSheet(bookingsListStyle);
    bookingsContainerLayout->addWidget(m_bookingsList);


    QHBoxLayout* bookingManagementLayout = new QHBoxLayout();
    bookingManagementLayout->setSpacing(16);

    m_cancelButton = new QPushButton(tr("Cancel Selected Booking"), bookingsContainer);
    m_cancelButton->setEnabled(true);
    m_cancelButton->setStyleSheet(cancelButtonStyle);
    bookingManagementLayout->addWidget(m_cancelButton);

    m_rescheduleButton = new QPushButton(tr("Reschedule Selected Booking"), bookingsContainer);
    m_rescheduleButton->setEnabled(false);
    m_rescheduleButton->setStyleSheet(rescheduleButtonStyle);
    bookingManagementLayout->addWidget(m_rescheduleButton);

    m_rescheduleTimeSelector = new QComboBox(bookingsContainer);
    m_rescheduleTimeSelector->setEnabled(false);
    m_rescheduleTimeSelector->setStyleSheet(rescheduleTimeSelectorStyle);
    bookingManagementLayout->addWidget(m_rescheduleTimeSelector);
    
    bookingsContainerLayout->addLayout(bookingManagementLayout);
    section3Layout->addWidget(bookingsContainer);

    section3Layout->addStretch();
    m_contentStack->addWidget(scroll1);
    m_contentStack->addWidget(scroll2);
    m_contentStack->addWidget(scroll3);
    rootLayout->addWidget(m_leftSidebar);
    rootLayout->addWidget(m_contentStack, 1);
    m_leftSidebar->setActiveButton("booking-section");
    m_contentStack->setCurrentWidget(scroll1);
}

void BookingWindow::handleSidebarPageChange(const QString& pageId)
{
    if (pageId == "booking-section") {
        m_contentStack->setCurrentIndex(0);
    } else if (pageId == "alternative-section") {
        m_contentStack->setCurrentIndex(1);
    } else if (pageId == "mybookings-section") {
        m_contentStack->setCurrentIndex(2);
    }
}

QFrame* BookingWindow::createSeparator()
{
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setLineWidth(1);
    return line;
}

void BookingWindow::setupConnections()
{
    connect(m_courtSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BookingWindow::onCourtSelectionChanged);
    connect(m_dateSelector, &QDateEdit::dateChanged, this, &BookingWindow::onDateChanged);
    connect(m_bookingsList, &QListWidget::currentRowChanged, this, &BookingWindow::onBookingItemSelected);
    connect(m_cancelButton, &QPushButton::clicked, this, &BookingWindow::cancelBooking);
    connect(m_rescheduleButton, &QPushButton::clicked, this, &BookingWindow::rescheduleBooking);

    connect(m_searchButton, &QPushButton::clicked, this, &BookingWindow::performSearch);
    connect(m_clearSearchButton, &QPushButton::clicked, this, &BookingWindow::clearSearch);
    connect(m_searchBox, &QLineEdit::returnPressed, this, &BookingWindow::performSearch);
    connect(m_locationFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BookingWindow::updateAlternativeCourts);
    connect(m_showAvailableOnly, &QCheckBox::checkStateChanged, this, &BookingWindow::updateAlternativeCourts);

    connect(m_joinWaitlistButton, &QPushButton::clicked, this, &BookingWindow::onJoinWaitlistClicked);
    connect(m_checkWaitlistButton, &QPushButton::clicked, this, &BookingWindow::onCheckWaitlistClicked);
}

void BookingWindow::loadCourts()
{
    m_courtSelector->clear();
    
    updateLocationFilter();
    
    QVector<Court> courts = m_padelManager->getAllCourts();
    for (const Court& court : courts) {
        m_courtSelector->addItem(court.getName(), court.getId());
    }
    
    if (m_courtSelector->count() > 0) {
        onCourtSelectionChanged(0);
    }
    
    m_totalResultsLabel->setText(tr("Total courts: %1").arg(courts.size()));
}

void BookingWindow::updateLocationFilter()
{
    QString currentSelection = m_locationFilter->currentData().toString();
    
    m_locationFilter->clear();
    m_locationFilter->addItem(tr("All Locations"), "");
    
    QSet<QString> locations;
    QVector<Court> courts = m_padelManager->getAllCourts();
    
    for (const Court& court : courts) {
        locations.insert(court.getLocation());
    }

    for (const QString& location : locations) {
        m_locationFilter->addItem(location, location);
    }

    if (!currentSelection.isEmpty()) {
        for (int i = 0; i < m_locationFilter->count(); i++) {
            if (m_locationFilter->itemData(i).toString() == currentSelection) {
                m_locationFilter->setCurrentIndex(i);
                break;
            }
        }
    }
}

void BookingWindow::performSearch()
{
    QString nameQuery = m_searchBox->text().trimmed();
    QString location = m_locationFilter->currentData().toString();
    bool availableOnly = m_showAvailableOnly->isChecked();
    
    loadCourtsByFilter(nameQuery, location, availableOnly);
    
    updateAlternativeCourts();
}

void BookingWindow::clearSearch()
{
    m_searchBox->clear();
    m_locationFilter->setCurrentIndex(0); 
    m_showAvailableOnly->setChecked(false);

    loadCourts();
    
    updateAlternativeCourts();
}

void BookingWindow::loadCourtsByFilter(const QString& nameQuery, const QString& location, bool availableOnly)
{
    try {
        qDebug() << "Loading courts by filter - NameQuery:" << nameQuery << "Location:" << location << "AvailableOnly:" << availableOnly;

        if (!m_courtSelector || !m_dateSelector) {
            qDebug() << "UI components are null in loadCourtsByFilter";
            return;
        }
        
        QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            selectedDate = timeLogicInstance.getCurrentTime().date();
        }

        QTime selectedTime;
        bool hasValidTimeSelection = false;
        
        if (m_timeSlotSelector && m_timeSlotSelector->count() > 0 && m_timeSlotSelector->currentIndex() >= 0) {
            QString timeStr = m_timeSlotSelector->currentText();
            selectedTime = QTime::fromString(timeStr, "HH:mm");
            hasValidTimeSelection = selectedTime.isValid();
        }
        
        if (!hasValidTimeSelection) {
            selectedTime = QTime(12, 0);
        }
        
        QDateTime requestedDateTime(selectedDate, selectedTime);
        qDebug() << "Selected date/time:" << requestedDateTime.toString();

        m_courtSelector->blockSignals(true);
        m_courtSelector->clear();

        QVector<Court> filteredCourts;
        
        if (availableOnly && hasValidTimeSelection) {
            
            filteredCourts = m_padelManager->getAvailableCourts(requestedDateTime, requestedDateTime.addSecs(3600), location);
    } else {
            
            if (!location.isEmpty()) {
            filteredCourts = m_padelManager->getCourtsByLocation(location);
        } else {
                filteredCourts = m_padelManager->getAllCourts();
            }

            if (!nameQuery.isEmpty()) {
                QVector<Court> nameFilteredCourts;
                for (const Court& court : filteredCourts) {
                    if (court.getName().contains(nameQuery, Qt::CaseInsensitive)) {
                        nameFilteredCourts.append(court);
                    }
                }
                filteredCourts = nameFilteredCourts;
            }
        }

    for (const Court& court : filteredCourts) {
            QVector<QTime> availableSlots;
            
            QJsonObject courtDetails = m_padelManager->getCourtDetails(court.getId());
            int maxAttendees = 2;
            if (courtDetails.contains("maxAttendees")) {
                maxAttendees = courtDetails["maxAttendees"].toInt();
            }
            
            if (maxAttendees <= 0) {
                maxAttendees = 2;
            }
            
            try {
                QJsonArray availableSlotsJson = m_padelManager->getAvailableTimeSlots(court.getId(), selectedDate, maxAttendees);
                
                for (int i = 0; i < availableSlotsJson.size(); i++) {
                    QJsonObject slotObject = availableSlotsJson[i].toObject();
                    QString startTimeStr = slotObject["startTime"].toString();
                    QTime startTime = QTime::fromString(startTimeStr, "HH:mm");
                    
                    if (startTime.isValid()) {
                        availableSlots.append(startTime);
                    }
                }
            } catch (const std::exception& e) {
                qDebug() << "Exception getting available slots:" << e.what();
            } catch (...) {
                qDebug() << "Unknown error getting available slots";
            }
            
            QString displayText = court.getName();

            if (!availableSlots.isEmpty()) {
                displayText += QString(" (%1 slots)").arg(availableSlots.size());
            }

            displayText += QString(" - $%1/hr").arg(court.getPricePerHour());

            m_courtSelector->addItem(displayText, court.getId());
        }

        m_courtSelector->blockSignals(false);

    if (m_courtSelector->count() > 0) {
            m_courtSelector->setCurrentIndex(0);
        onCourtSelectionChanged(0);
        } else {
            clearTimeSlotGrid();
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in loadCourtsByFilter:" << e.what();
    } catch (...) {
        qDebug() << "Unknown error in loadCourtsByFilter";
    }
}

void BookingWindow::onJoinWaitlistClicked()
{
    try {
    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("User Error"), 
                          tr("Please log in to join the waitlist."));
        return;
    }
        
        if (!m_padelManager) {
            qDebug() << "Error: PadelDataManager is null";
            QMessageBox::warning(this, tr("System Error"), 
                              tr("System error: Unable to access data manager."));
        return;
    }
    
    if (m_courtSelector->count() <= 0) {
        QMessageBox::warning(this, tr("Selection Error"), 
                          tr("Please select a court."));
        return;
    }

    int courtId = m_courtSelector->currentData().toInt();
        if (courtId <= 0) {
            QMessageBox::warning(this, tr("Selection Error"), 
                              tr("Invalid court selection."));
            return;
        }
        
    QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            selectedDate = timeLogicInstance.getCurrentTime().date();
        }

        QTime selectedTime;
        if (m_timeSlotSelector->count() > 0) {
            QString timeStr = m_timeSlotSelector->currentText();
            selectedTime = QTime::fromString(timeStr, "HH:mm");
            if (!selectedTime.isValid()) {
                selectedTime = QTime(12, 0); 
            }
        } else {
            selectedTime = QTime(12, 0);   
        }
        
        QDateTime requestedDateTime(selectedDate, selectedTime);
        
        qDebug() << "========== JOIN WAITLIST FLOW ==========";
        qDebug() << "Starting waitlist join process for user:" << m_currentUserId;
        qDebug() << "Court ID:" << courtId;
        qDebug() << "Selected date:" << selectedDate.toString("yyyy-MM-dd");
        qDebug() << "Selected time:" << selectedTime.toString("HH:mm");

        bool isInWaitlist = false;
        try {
            isInWaitlist = m_padelManager->isUserInWaitlist(m_currentUserId, courtId, requestedDateTime);
            qDebug() << "User" << m_currentUserId << "is in waitlist for court" << courtId << ":" << isInWaitlist;
        
        if (isInWaitlist) {
                QMessageBox::StandardButton response = QMessageBox::question(
                    this, tr("Leave Waitlist"),
                    tr("You are already on the waitlist for this court. Would you like to be removed?"),
                    QMessageBox::Yes | QMessageBox::No
                );
                
                if (response == QMessageBox::Yes) {
            QString errorMessage;
                    bool success = m_padelManager->removeFromWaitlist(m_currentUserId, courtId, errorMessage);
            
            if (success) {
                        QMessageBox::information(this, tr("Success"), 
                                     tr("You have been removed from the waitlist."));
                        
                updateWaitlistStatus(courtId);
                        return;
            } else {
                QMessageBox::warning(this, tr("Error"), 
                                  tr("Failed to remove from waitlist: %1").arg(errorMessage));
            return;
        }
                } else {
                    return; 
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception checking waitlist status:" << e.what();
            QMessageBox::warning(this, tr("System Error"), 
                              tr("System error: Failed to check waitlist status."));
            return;
        }
        
        try {
            QJsonObject courtDetails = m_padelManager->getCourtDetails(courtId);
            int maxAttendees = 2; 
            if (courtDetails.contains("maxAttendees")) {
                maxAttendees = courtDetails["maxAttendees"].toInt();
            }
            
            if (maxAttendees <= 0) {
                maxAttendees = 2;
            }
            
            QDateTime startDateTime(selectedDate, selectedTime);
            QDateTime endDateTime = startDateTime.addSecs(3600); 
            
            bool isAvailable = m_padelManager->isCourtAvailable(courtId, startDateTime, endDateTime);
            qDebug() << "Court" << courtId << "is available at" << startDateTime.toString() << ":" << isAvailable;
            
            if (isAvailable) {
                QMessageBox::StandardButton response = QMessageBox::question(
                    this, tr("Court Available"),
                    tr("There are still available slots for this court. Do you want to book directly instead of joining waitlist?"),
                    QMessageBox::Yes | QMessageBox::No
                );
                
                qDebug() << "User response to direct booking option:" << (response == QMessageBox::Yes ? "Yes - Book directly" : "No - Continue with waitlist");
                
                if (response == QMessageBox::Yes) {
                    qDebug() << "User chose to book directly - calling bookCourtDirectly()";
                    bookCourtDirectly(courtId, startDateTime, endDateTime);
        return;
                }
                qDebug() << "User chose to continue with waitlist despite available slots";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception checking court availability:" << e.what();
    }

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Join Waitlist"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* promptLabel = new QLabel(tr("Please select your preferred time for the waitlist:"), &dialog);
    QTimeEdit* timeEdit = new QTimeEdit(&dialog);
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setTime(QTime(12, 0));   
    
    QPushButton* confirmButton = new QPushButton(tr("Join Waitlist"), &dialog);
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), &dialog);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(promptLabel);
    layout->addWidget(timeEdit);
    layout->addLayout(buttonLayout);
    
    connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    
    qDebug() << "Showing preferred time dialog to user";
    if (dialog.exec() == QDialog::Accepted) {
        QTime preferredTime = timeEdit->time();
            m_selectedWaitlistTime = preferredTime;
        
        qDebug() << "User confirmed preferred time:" << preferredTime.toString() << " - calling joinWaitlist()";
        
        joinWaitlist(courtId);
        } else {
            qDebug() << "User cancelled waitlist join operation";
        }
        qDebug() << "========== END JOIN WAITLIST FLOW ==========";
    } catch (const std::exception& e) {
        qDebug() << "Exception in onJoinWaitlistClicked:" << e.what();
        QMessageBox::warning(this, tr("System Error"), 
                          tr("System error: An error occurred while joining the waitlist."));
    } catch (...) {
        qDebug() << "Unknown exception in onJoinWaitlistClicked";
        QMessageBox::warning(this, tr("System Error"), 
                          tr("System error: An unknown error occurred while joining the waitlist."));
    }
}

void BookingWindow::joinWaitlist(int courtId)
{
        if (!m_padelManager) {
        QMessageBox::warning(this, tr("Error"), tr("System error: PadelDataManager is null."));
            return;
        }
        
    try {
        qDebug() << "=== JOIN WAITLIST FUNCTION ===";
        qDebug() << "Join waitlist for court:" << courtId;
        
    QDate selectedDate = m_dateSelector->date();
        QTime preferredTime = m_selectedWaitlistTime.isValid() ? m_selectedWaitlistTime : QTime(12, 0);
        
        qDebug() << "Selected waitlist time:" << (m_selectedWaitlistTime.isValid() ? "Valid: " + m_selectedWaitlistTime.toString() : "Invalid - using default noon");
        
        if (!preferredTime.isValid() && m_timeSlotSelector && m_timeSlotSelector->count() > 0 && m_timeSlotSelector->currentIndex() >= 0) {
        QString timeStr = m_timeSlotSelector->currentText();
            QTime parsedTime = QTime::fromString(timeStr, "HH:mm");
            if (parsedTime.isValid()) {
                preferredTime = parsedTime;
                qDebug() << "Using time from selector:" << preferredTime.toString();
            }
        }
        
        qDebug() << "Attempting to join waitlist for time:" << preferredTime.toString("HH:mm");
    
    QDateTime requestedTime(selectedDate, preferredTime);
    
        int userId = m_currentUserId;
        if (userId <= 0) {
            QMessageBox::warning(this, tr("Error"), tr("You must be logged in to join the waitlist."));
                return;
            }
        
        qDebug() << "Checking if user" << userId << "is already in waitlist";
        bool isInWaitlist = false;
        try {
            isInWaitlist = m_padelManager->isUserInWaitlist(userId, courtId, requestedTime);
            qDebug() << "User" << userId << "is in waitlist for court" << courtId << ":" << isInWaitlist;
        } catch (const std::exception& e) {
            qDebug() << "Exception checking waitlist status:" << e.what();
            isInWaitlist = false; 
        }
        
        if (isInWaitlist) {
            
            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, tr("Confirm Removal"),
                tr("Are you sure you want to be removed from the waitlist?"),
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (confirm == QMessageBox::Yes) {
                QString errorMessage;
                bool success = m_padelManager->removeFromWaitlist(userId, courtId, errorMessage);
                
                if (success) {
                    QMessageBox::information(this, tr("Success"), 
                        tr("You have been removed from the waitlist."));
                    updateWaitlistStatus(courtId);
                    } else {
                    QMessageBox::warning(this, tr("Error"), 
                        tr("Failed to remove from waitlist: %1").arg(errorMessage));
                }
                    }
                } else {
            qDebug() << "Checking if user" << userId << "has booking on date" << selectedDate.toString();
            bool hasBooking = false;
            try {
                hasBooking = m_padelManager->userHasBookingOnDate(userId, courtId, selectedDate);
                qDebug() << "User has existing booking:" << hasBooking;
            } catch (const std::exception& e) {
                qDebug() << "Exception checking booking status:" << e.what();
                hasBooking = false; 
            }
            
            if (hasBooking) {
                QMessageBox::warning(this, tr("Error"), 
                    tr("You already have a booking for this court on this date."));
            return;
        }

            QMessageBox::StandardButton confirm = QMessageBox::question(
                this, tr("Confirm Waitlist"),
                tr("Would you like to be added to the waitlist for this court at") + " " + preferredTime.toString("HH:mm") + "?",
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (confirm == QMessageBox::Yes) {
    QString errorMessage;
                
                qDebug() << "=== WAITLIST JOIN DETAILS ===";
                qDebug() << "User ID:" << userId;
                qDebug() << "Court ID:" << courtId;
                qDebug() << "Selected Date:" << selectedDate.toString("yyyy-MM-dd");
                qDebug() << "Preferred Time:" << preferredTime.toString("HH:mm");
                qDebug() << "RequestedTime:" << requestedTime.toString("yyyy-MM-dd HH:mm:ss");
                qDebug() << "===========================";
                
                qDebug() << "Calling addToWaitlist function...";
        bool success = false;
        
        try {
                    success = m_padelManager->addToWaitlist(userId, courtId, requestedTime, errorMessage);
                    qDebug() << "Add to waitlist result:" << (success ? "SUCCESS" : "FAILED");
                    if (!success) {
                        qDebug() << "Error message:" << errorMessage;
                    }
        } catch (const std::exception& e) {
                    qDebug() << "Exception in addToWaitlist call:" << e.what();
                    success = false;
        } catch (...) {
                    qDebug() << "Unknown exception in addToWaitlist call";
                    success = false;
                }
                QMessageBox::information(this, tr("Success"), 
                    tr("You have been added to the waitlist. You will be notified if a slot becomes available."));
                try {
                    qDebug() << "Manually updating waitlist UI...";
                    if (m_waitlistStatusLabel) {
                        m_waitlistStatusLabel->setText(tr("You are on the waitlist for") + " " + 
                                                     preferredTime.toString("HH:mm"));
                        m_waitlistStatusLabel->setStyleSheet("color: blue;");
                    }
                    
                    qDebug() << "Refreshing time slots to update UI after waitlist join";
            refreshTimeSlots();
                } catch (const std::exception& e) {
                    qDebug() << "Exception updating UI:" << e.what();
                }
    } else {
                qDebug() << "User canceled waitlist join confirmation";
        }
        }
        
        qDebug() << "=== END JOIN WAITLIST FUNCTION ===";
    } catch (const std::exception& e) {
        qDebug() << "Exception in joinWaitlist:" << e.what();
        QMessageBox::warning(this, tr("Error"), 
            tr("An exception occurred: %1").arg(e.what()));
    } catch (...) {
        qDebug() << "Unknown exception in joinWaitlist";
        QMessageBox::warning(this, tr("Error"), 
            tr("An unknown error occurred while processing your request."));
    }
}

void BookingWindow::onCheckWaitlistClicked()
{
    try {
    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("User Error"), 
                           tr("Please log in to check waitlist status."));
        return;
    }
    
        if (!m_padelManager) {
            qDebug() << "Error: PadelDataManager is null in onCheckWaitlistClicked";
            QMessageBox::warning(this, tr("System Error"), 
                               tr("System error: Unable to access data manager."));
            return;
        }

        int courtId = m_courtSelector->currentData().toInt();
        if (courtId <= 0) {
            QMessageBox::warning(this, tr("Selection Error"), 
                               tr("Invalid court selection."));
            return;
        }
        
        QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            selectedDate = timeLogicInstance.getCurrentTime().date();
        }
        
        QJsonObject waitlistInfo;
        try {
            waitlistInfo = m_padelManager->getDetailedWaitlistInfo(courtId, selectedDate);
        } catch (const std::exception& e) {
            qDebug() << "Exception getting detailed waitlist info:" << e.what();
            QMessageBox::warning(this, tr("Waitlist Error"), 
                               tr("Error retrieving waitlist information: %1").arg(e.what()));
            return;
        } catch (...) {
            qDebug() << "Unknown exception getting detailed waitlist info";
            QMessageBox::warning(this, tr("Waitlist Error"), 
                               tr("Error retrieving waitlist information."));
        return;
    }
    
        if (waitlistInfo.contains("error")) {
            QMessageBox::warning(this, tr("Waitlist Error"), 
                               waitlistInfo["error"].toString());
            return;
        }
        
        int waitlistCount = waitlistInfo["waitlistCount"].toInt();
        
        if (waitlistCount == 0) {
        QMessageBox::information(this, tr("Waitlist Status"), 
                                  tr("There are no users on the waitlist for this court on the selected date."));
        return;
    }
    
    QDialog dialog(this);
        dialog.setWindowTitle(tr("Waitlist Information"));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
        Court court;
        try {
            court = m_padelManager->getCourtById(courtId);
            if (court.getId() <= 0) {
                QMessageBox::warning(this, tr("Court Error"), 
                                   tr("Could not retrieve court information."));
                return;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception retrieving court info:" << e.what();
            QMessageBox::warning(this, tr("Court Error"), 
                               tr("Could not retrieve court information: %1").arg(e.what()));
            return;
        } catch (...) {
            qDebug() << "Unknown exception retrieving court info";
            QMessageBox::warning(this, tr("Court Error"), 
                               tr("Could not retrieve court information."));
            return;
        }
        
        QLabel* courtLabel = new QLabel(tr("<b>Court:</b> %1").arg(court.getName()), &dialog);
        QLabel* dateLabel = new QLabel(tr("<b>Date:</b> %1").arg(selectedDate.toString("yyyy-MM-dd")), &dialog);
        QLabel* countLabel = new QLabel(tr("<b>Total users on waitlist:</b> %1").arg(waitlistCount), &dialog);
        
        layout->addWidget(courtLabel);
        layout->addWidget(dateLabel);
        layout->addWidget(countLabel);

        QFrame* separator = new QFrame(&dialog);
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        layout->addWidget(separator);

    QTableWidget* table = new QTableWidget(&dialog);
    table->setColumnCount(4);
        table->setHorizontalHeaderLabels({tr("Position"), tr("User"), tr("Requested Time"), tr("VIP Status")});

        QJsonArray entries = waitlistInfo["entries"].toArray();
        table->setRowCount(entries.size());
        
        try {
            for (int i = 0; i < entries.size(); i++) {
                QJsonObject entry = entries[i].toObject();
                
                QString userName = entry.contains("userName") ? 
                                entry["userName"].toString() : 
                                tr("User %1").arg(entry["userId"].toInt());

                bool isCurrentUser = (entry["userId"].toInt() == m_currentUserId);
                
                table->setItem(i, 0, new QTableWidgetItem(QString::number(entry["position"].toInt())));
                table->setItem(i, 1, new QTableWidgetItem(userName));

                QString timeStr = entry["requestedTime"].toString();
                QString displayTime;
                
                if (timeStr.contains("yyyy-MM-dd HH:mm")) {
                    displayTime = QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm").time().toString("HH:mm");
        } else {
                    int timeIndex = timeStr.lastIndexOf(' ');
                    if (timeIndex > 0 && timeIndex < timeStr.length() - 1) {
                        displayTime = timeStr.mid(timeIndex + 1);
                    } else {
                        displayTime = timeStr; 
                    }
                }
                
                table->setItem(i, 2, new QTableWidgetItem(displayTime));
                table->setItem(i, 3, new QTableWidgetItem(entry["isVIP"].toBool() ? tr("Yes") : tr("No")));
                
                if (isCurrentUser) {
                    for (int col = 0; col < table->columnCount(); col++) {
                        QTableWidgetItem* item = table->item(i, col);
                        if (item) {
                            item->setBackground(QBrush(QColor(255, 235, 205))); 
                            item->setFont(QFont("", -1, QFont::Bold));
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception populating waitlist table:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception populating waitlist table";
    }
    
    table->resizeColumnsToContents();
    layout->addWidget(table);
    
    QPushButton* closeButton = new QPushButton(tr("Close"), &dialog);
    layout->addWidget(closeButton);
    
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    
    dialog.setMinimumWidth(500);
    dialog.exec();
    } catch (const std::exception& e) {
        qDebug() << "Exception in onCheckWaitlistClicked:" << e.what();
        QMessageBox::warning(this, tr("System Error"), 
                          tr("System error: An error occurred while checking waitlist status."));
    } catch (...) {
        qDebug() << "Unknown exception in onCheckWaitlistClicked";
        QMessageBox::warning(this, tr("System Error"), 
                          tr("System error: An unknown error occurred while checking waitlist status."));
    }
}

int BookingWindow::getMyWaitlistPosition(int courtId)
{
    if (m_currentUserId <= 0 || courtId <= 0) {
        qDebug() << "Invalid userId or courtId in getMyWaitlistPosition";
        return -1;
    }
    
    if (!m_padelManager) {
        qDebug() << "PadelDataManager is null in getMyWaitlistPosition";
        return -1;
    }
    
    try {
        QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            selectedDate = timeLogicInstance.getCurrentTime().date();
        }
        
        QTime defaultTime(12, 0);   
        QDateTime requestedDateTime(selectedDate, defaultTime);
        
        if (!requestedDateTime.isValid()) {
            qDebug() << "Invalid requestedDateTime in getMyWaitlistPosition";
            return -1;
        }
        
        bool isInWaitlist = false;
        
        try {
            isInWaitlist = m_padelManager->isUserInWaitlist(m_currentUserId, courtId, requestedDateTime);
        } catch (const std::exception& e) {
            qDebug() << "Exception checking if user is in waitlist:" << e.what();
            isInWaitlist = false;
        } catch (...) {
            qDebug() << "Unknown exception checking if user is in waitlist";
            isInWaitlist = false;
        }
        
        if (!isInWaitlist) {
            return -1;
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in initial waitlist check:" << e.what();
    } catch (...) {
        qDebug() << "Unknown error in initial waitlist check";
    }
    
    try {
        
        Court court;
        try {
            court = m_padelManager->getCourtById(courtId);
            if (court.getId() <= 0) {
                qDebug() << "Court not found in getMyWaitlistPosition:" << courtId;
                return -1;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception getting court:" << e.what();
            return -1;
        } catch (...) {
            qDebug() << "Unknown exception getting court";
                return -1;
            }

            QDate selectedDate = m_dateSelector->date();
            if (!selectedDate.isValid()) {
                selectedDate = timeLogicInstance.getCurrentTime().date();
            }
            
        QJsonObject waitlistInfo;
        try {
            waitlistInfo = m_padelManager->getDetailedWaitlistInfo(courtId, selectedDate);

            if (waitlistInfo.contains("error") || waitlistInfo["waitlistCount"].toInt() == 0) {
                qDebug() << "No waitlist entries found for court" << courtId;
                return -1;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception getting waitlist info:" << e.what();
            return -1;
        } catch (...) {
            qDebug() << "Unknown exception getting waitlist info";
                return -1;
            }

        int position = -1;
        try {
            position = m_padelManager->getWaitlistPosition(m_currentUserId, courtId);
            return position > 0 ? position : -1;
        } catch (const std::exception& e) {
            qDebug() << "Exception getting waitlist position:" << e.what();
            return -1;
        } catch (...) {
            qDebug() << "Unknown exception getting waitlist position";
            return -1;
        }
    } catch (const std::exception& e) {
        qDebug() << "Critical error in getMyWaitlistPosition:" << e.what();
        return -1;
    } catch (...) {
        qDebug() << "Critical error in getMyWaitlistPosition";
        return -1;
    }
}

void BookingWindow::updateWaitlistStatus(int courtId)
{

    static bool isUpdating = false;
    if (isUpdating) return;
    
    isUpdating = true;
    
    try {
        qDebug() << "Updating waitlist status for court" << courtId;
        
        if (!m_padelManager || !m_waitlistStatusLabel || !m_joinWaitlistButton) {
            qDebug() << "Required components are null in updateWaitlistStatus";
            isUpdating = false;
            return;
        }
        
        QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            qDebug() << "Selected date is invalid in updateWaitlistStatus";
            isUpdating = false;
            return;
        }
        
        int userId = m_currentUserId;
        if (userId <= 0) {
            qDebug() << "Invalid user ID in updateWaitlistStatus:" << userId;
            m_waitlistStatusLabel->setText(tr("Please log in to join the waitlist"));
            m_joinWaitlistButton->setEnabled(false);
            isUpdating = false;
            return;
        }

        QDateTime requestedDateTime(selectedDate, QTime(12, 0)); 
        bool userInWaitlist = m_padelManager->isUserInWaitlist(userId, courtId, requestedDateTime);
        
        if (userInWaitlist) {
            qDebug() << "User is in waitlist";
            int position = m_padelManager->getWaitlistPosition(userId, courtId);
            
            if (position > 0) {
                m_waitlistStatusLabel->setText(tr("You are #%1 on the waitlist").arg(position));
                m_joinWaitlistButton->setText(tr("Leave Waitlist"));
            m_joinWaitlistButton->setEnabled(true);
                m_joinWaitlistButton->setToolTip(tr("Leave the waitlist for this court"));
        } else {
                m_waitlistStatusLabel->setText(tr("You are on the waitlist"));
                m_joinWaitlistButton->setText(tr("Leave Waitlist"));
                m_joinWaitlistButton->setEnabled(true);
                m_joinWaitlistButton->setToolTip(tr("Leave the waitlist for this court"));
            }
        } else {
            qDebug() << "User is not in waitlist";
            m_waitlistStatusLabel->setText("");
            m_joinWaitlistButton->setText(tr("Join Waitlist"));

            bool anyAvailableSlots = false;
            
            try {
                
                QJsonObject courtDetails = m_padelManager->getCourtDetails(courtId);
                int maxAttendees = 2; 
                if (courtDetails.contains("maxAttendees")) {
                    maxAttendees = courtDetails["maxAttendees"].toInt();
                }
                
                if (maxAttendees <= 0) {
                    maxAttendees = 2;
                }

                QJsonArray availableSlots = m_padelManager->getAvailableTimeSlots(courtId, selectedDate, maxAttendees);
                anyAvailableSlots = !availableSlots.isEmpty();
                qDebug() << "Available slots for court" << courtId << ":" << availableSlots.size();
            } catch (const std::exception& e) {
                qDebug() << "Exception getting available time slots:" << e.what();
                anyAvailableSlots = true;    
            } catch (...) {
                qDebug() << "Unknown error getting available time slots";
                anyAvailableSlots = true;    
            }
            
            m_joinWaitlistButton->setEnabled(!anyAvailableSlots);
            
            if (anyAvailableSlots) {
                m_joinWaitlistButton->setToolTip(tr("There are available slots. No need to join waitlist."));
            } else {
                m_joinWaitlistButton->setToolTip(tr("Join waitlist for this court"));
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception in updateWaitlistStatus:" << e.what();
        if (m_waitlistStatusLabel) m_waitlistStatusLabel->setText("");
        if (m_joinWaitlistButton) {
            m_joinWaitlistButton->setText(tr("Join Waitlist"));
            m_joinWaitlistButton->setEnabled(false);
        }
    } catch (...) {
        qDebug() << "Unknown exception in updateWaitlistStatus";
        if (m_waitlistStatusLabel) m_waitlistStatusLabel->setText("");
        if (m_joinWaitlistButton) {
            m_joinWaitlistButton->setText(tr("Join Waitlist"));
            m_joinWaitlistButton->setEnabled(false);
        }
    }

    isUpdating = false;
}

void BookingWindow::showPreviousWeek() {
    QDate newDate = m_dateSelector->date().addDays(-7);
    m_dateSelector->setDate(newDate);
    refreshTimeSlots();
}

void BookingWindow::showNextWeek() {
    QDate newDate = m_dateSelector->date().addDays(7);
    m_dateSelector->setDate(newDate);
    refreshTimeSlots();
}

void BookingWindow::updateWeekRangeLabel() {
    QDate selectedDate = m_dateSelector->date();
    QDate sundayDate = selectedDate.addDays(-(selectedDate.dayOfWeek() % 7));
    QDate saturdayDate = sundayDate.addDays(6);

    m_weekRangeLabel->setText(
        QString("Sun %1 - Sat %2")
        .arg(sundayDate.toString("d MMM"), saturdayDate.toString("d MMM"))
    );
}

void BookingWindow::refreshTimeSlots() {
    updateWeekRangeLabel();
    QDate selectedDate = m_dateSelector->date();
    QDate sundayDate = selectedDate.addDays(-(selectedDate.dayOfWeek()%7)); // Start from Sunday

    // First reset all cells
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 15; ++col) {
            if (auto* cell = qobject_cast<CalendarButton*>(m_calendarCells[row][col])) {
                cell->resetState();
            }
        }
    }

    QStringList dayNames = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    int courtId = m_courtSelector->currentData().toInt();
    if (courtId <= 0) return;

    for (int day = 0; day < 7; day++) {
        QDate currentDate = sundayDate.addDays(day);
        bool isSelectedDay = (currentDate == selectedDate);

        if (m_dayHeaders[day]) {
            m_dayHeaders[day]->setText(QString("%1\n%2")
                 .arg(dayNames[day], currentDate.toString("d/M")));

            m_dayHeaders[day]->setStyleSheet(calendarDayLabelStyle.arg(isSelectedDay ? "#7C3AED" : 
                                                                       m_isDarkTheme ? "#9CAFF0" : "#33498F" , (m_isDarkTheme ? "black" : "white")));
        }
        
        for (int timeSlot = 0; timeSlot < 15; timeSlot++) {
            CalendarButton* cellButton = qobject_cast<CalendarButton*>(m_calendarCells[day][timeSlot]);
            if (!cellButton) continue;

            // Calculate time for this cell
            QTime startTime = QTime(7, 0).addSecs(timeSlot * 3600);
            QTime endTime = startTime.addSecs(3600);
            QDateTime startDateTime(currentDate, startTime);
            QDateTime endDateTime(currentDate, endTime);

            bool isBookedByUser = false;
            bool isFullyBooked = false;
            int bookingId = -1;
            int currentAttendees = 0;
            int maxAttendees = m_padelManager->getCourtById(courtId).getMaxAttendees();
            QVector<Booking> slotBookings = m_padelManager->getBookingsForTimeSlot(courtId, startDateTime, endDateTime);
            
            // Count valid (non-cancelled) bookings
            for (const Booking& booking : slotBookings) {
                if (!booking.isCancelled()) {
                    currentAttendees++;
                    if (booking.getUserId() == m_currentUserId) {
                        isBookedByUser = true;
                        bookingId = booking.getBookingId();
                    }
                }
            }    

            isFullyBooked = (currentAttendees >= maxAttendees);
            
            // First disconnect any existing connections to prevent signal/slot issues
            cellButton->disconnect();

            QString timeText = QString("%1 - %2").arg(
                    startTime.toString("h:mm AP"),
                    endTime.toString("h:mm AP")
                );
            QString availabilityText = QString("%1/%2").arg(currentAttendees).arg(maxAttendees);

            // Highlight selected day's cells
            cellButton->setProperty("isSelectedDay", isSelectedDay);

            if (isBookedByUser) {
                // User's own booking - show cancel option
                cellButton->setBooked(true, bookingId);
                m_selectedBookingId = bookingId;
                connect(cellButton, &CalendarButton::clicked, this, &BookingWindow::cancelBooking);
            } else if (isFullyBooked) {
                // Fully booked by others
                cellButton->setSlotInfo(timeText, availabilityText, false);
                
                // Add waitlist option on click for full slots
                connect(cellButton, &CalendarButton::clicked, [this, courtId, startDateTime]() {
                    QMessageBox msgBox(this);
                    msgBox.setWindowTitle(tr("Court At Capacity"));
                    msgBox.setText(tr("The court is at maximum capacity for this time slot.\nWhat would you like to do?"));
                    msgBox.setInformativeText(tr("VIP members receive priority on the waitlist."));
                    
                    QPushButton* waitlistButton = msgBox.addButton(tr("Join Waitlist"), QMessageBox::ActionRole);
                    QPushButton* alternativeButton = msgBox.addButton(tr("Check Alternative Courts"), QMessageBox::ActionRole);
                    QPushButton* closeButton = msgBox.addButton(QMessageBox::Close);
                    
                    msgBox.setDefaultButton(waitlistButton);
                    msgBox.setEscapeButton(closeButton);
                    
                    msgBox.exec();
                    
                    if (msgBox.clickedButton() == waitlistButton) {
                        m_selectedWaitlistTime = startDateTime.time();
                        joinWaitlist(courtId);
                    } else if (msgBox.clickedButton() == alternativeButton) {
                        handleSidebarPageChange("alternative-section");
                        updateAlternativeCourts();
                    }
                });
            } else {
                // Available slot - show booking option
                cellButton->setSlotInfo(timeText, availabilityText, true);
                connect(cellButton, &CalendarButton::clicked, [this, courtId, startDateTime, endDateTime]() {
                    bookCourtDirectly(courtId, startDateTime, endDateTime);
                });
            }

            cellButton->setStyleSheet(cellButton->normalStyle());
        }
    }
}

void BookingWindow::onBookButtonClicked(int courtId, const QDate& date, const QTime& startTime, const QTime& endTime)
{
    try {
        qDebug() << "Book button clicked for court" << courtId << "on" << date.toString() 
                << "from" << startTime.toString() << "to" << endTime.toString();

        if (courtId <= 0 || !date.isValid() || !startTime.isValid() || !endTime.isValid()) {
            qDebug() << "Invalid parameters in onBookButtonClicked";
            QMessageBox::warning(this, tr("Error"), tr("Invalid booking parameters. Please try again."));
                                return;
                            }
                            
        if (m_currentUserId <= 0) {
            qDebug() << "User not logged in when attempting to book";
            QMessageBox::information(this, tr("Login Required"), 
                                  tr("You need to be logged in to book a court. Please log in and try again."));
            return;
        }
        
        QDateTime startDateTime(date, startTime);
        QDateTime endDateTime(date, endTime);
        
        QString message = tr("Do you want to book court %1 on %2 from %3 to %4?")
                         .arg(courtId)
                         .arg(date.toString("dd/MM/yyyy"))
                         .arg(startTime.toString("hh:mm"))
                         .arg(endTime.toString("hh:mm"));
                         
        int result = QMessageBox::question(this, tr("Confirm Booking"), message, 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
        if (result == QMessageBox::Yes) {
            bookCourtDirectly(courtId, startDateTime, endDateTime);
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in onBookButtonClicked:" << e.what();
        QMessageBox::warning(this, tr("Error"), tr("An error occurred while booking. Please try again."));
    }
    catch (...) {
        qDebug() << "Unknown exception in onBookButtonClicked";
        QMessageBox::warning(this, tr("Error"), tr("An error occurred while booking. Please try again."));
    }
}

void BookingWindow::clearTimeSlotGrid() const {
    try {
        qDebug() << "Clearing time slot grid";

        // First check if the calendar grid exists
        if (!m_calendarGrid) {
            qDebug() << "Calendar grid is null in clearTimeSlotGrid";
            return;
        }

        // Clear all calendar cells with proper null checks
        for (int day = 0; day < 7; day++) {
            for (int timeSlot = 0; timeSlot < 15; timeSlot++) {
                // Check if the cell exists in the 2D array
                if (day >= 0 && day < 7 && timeSlot >= 0 && timeSlot < 15) {
                    QWidget* cellWidget = m_calendarCells[day][timeSlot];
                    if (cellWidget) {
                        CalendarButton* button = qobject_cast<CalendarButton*>(cellWidget);
                        if (button) {
                            // Disconnect all signals from this button
                            button->disconnect();
                            // Clear the button's content
                            button->setSlotInfo("", "", false);
                        }
                    }
                }
            }
        }
        
        qDebug() << "Time slot grid cleared successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception in clearTimeSlotGrid:" << e.what();
    }
    catch (...) {
        qDebug() << "Unknown exception in clearTimeSlotGrid";
    }
}

void BookingWindow::onCourtSelectionChanged(int index)
{
    try {
        if (index < 0 || m_courtSelector->count() <= index) {
            return;
        }
        
        int courtId = m_courtSelector->itemData(index).toInt();
        if (courtId <= 0) {
            qDebug() << "WARNING: Invalid court ID at index" << index;
            return;
        }
        
        //updateCourtDetails(courtId);
        refreshTimeSlots();
        //updateWaitlistStatus(courtId);
        updateAlternativeCourts();
    }
    catch (const std::exception& e) {
        qDebug() << "ERROR in onCourtSelectionChanged:" << e.what();
    }
    catch (...) {
        qDebug() << "UNKNOWN ERROR in onCourtSelectionChanged";
    }
}

void BookingWindow::onDateChanged(const QDate& date)
{
    refreshTimeSlots();
    
    if (m_courtSelector->count() > 0) {
        int courtId = m_courtSelector->currentData().toInt();
        //updateCourtDetails(courtId);
    }

    //updateAlternativeCourts();
}

void BookingWindow::bookCourtDirectly(int courtId, const QDateTime& startTime, const QDateTime& endTime)
{
    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("User Error"), 
                           tr("Please log in to book a court."));
        return;
    }

    Court court = m_padelManager->getCourtById(courtId);
    if (court.getId() <= 0 || !startTime.isValid() || !endTime.isValid() || startTime < timeLogicInstance.getCurrentTime()) {
        QMessageBox::warning(this, tr("Court Error"), 
                           tr("Invalid court selected."));
        return;
    }

    if (!m_padelManager->isCourtAvailable(courtId, startTime, endTime)) {
        int currentAttendees = m_padelManager->getCurrentAttendees(courtId, startTime, endTime);
        int maxAttendees = court.getMaxAttendees();
        
        Court alternativeCourt = m_padelManager->findClosestAvailableCourt(courtId, startTime, endTime);
        
        if (alternativeCourt.getId() > 0) {
            QString errorMessage;
            if (currentAttendees >= maxAttendees) {
                errorMessage = tr("Court is at maximum capacity (%1/%2 attendees).").arg(currentAttendees).arg(maxAttendees);
            } else {
                errorMessage = tr("Court is not available at the selected time.");
            }
            
            QString suggestionMessage = errorMessage + "\n\n" +
                tr("Would you like to book court %1 at %2 instead? It's available at the same time.")
                .arg(alternativeCourt.getName())
                .arg(alternativeCourt.getLocation());
            
            QMessageBox::StandardButton response = QMessageBox::question(
                this, tr("Alternative Court Available"), 
                suggestionMessage,
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (response == QMessageBox::Yes) {
                int alternativeCourtId = alternativeCourt.getId();
                
                for (int i = 0; i < m_courtSelector->count(); i++) {
                    if (m_courtSelector->itemData(i).toInt() == alternativeCourtId) {
                        m_courtSelector->setCurrentIndex(i);
                        break;
                    }
                }
                
                bookCourtDirectly(alternativeCourtId, startTime, endTime);
                return;
            } else {
                if (errorMessage.contains("maximum capacity")) {
                    QMessageBox::StandardButton response = QMessageBox::question(
                        this, 
                        tr("Court At Capacity"),
                        tr("The court is at maximum capacity for this time slot.\nWould you like to join the waitlist? If someone cancels, you'll be automatically notified.\n\nVIP members receive priority on the waitlist."),
                        QMessageBox::Yes | QMessageBox::No
                    );
                    
                    if (response == QMessageBox::Yes) {
                        m_selectedWaitlistTime = startTime.time();
                        joinWaitlist(courtId);
                    }
                } else {
                    QMessageBox::warning(this, tr("Availability Error"), errorMessage);
                }

                //updateCourtDetails(courtId);
                refreshTimeSlots();
                updateWaitlistStatus(courtId);
                return;
            }
        } else {
            QString errorMessage;
            if (currentAttendees >= maxAttendees) {
                errorMessage = tr("Court is at maximum capacity (%1/%2 attendees).").arg(currentAttendees).arg(maxAttendees);
            } else {
                errorMessage = tr("Court is not available at the selected time.");
            }
            
            QMessageBox::warning(this, tr("Availability Error"), errorMessage);
                return;
        }
    }

    QString confirmMessage = tr("You are about to book:\n\n") +
                          tr("Court: %1\n").arg(court.getName()) +
                          tr("Date: %1\n").arg(startTime.date().toString("yyyy-MM-dd")) +
                          tr("Time: %1 - %2\n\n").arg(startTime.time().toString("HH:mm")).arg(endTime.time().toString("HH:mm")) +
                          tr("Continue?");
                          
    QMessageBox::StandardButton response = QMessageBox::question(
        this, tr("Confirm Booking"), confirmMessage, QMessageBox::Yes | QMessageBox::No);
        
    if (response != QMessageBox::Yes) {
            return;
        }

    QString errorMessage;
    bool success = m_padelManager->createBooking(m_currentUserId, courtId, startTime, endTime, errorMessage);
        
        if (success) {
            NotificationManager::instance().showNotification(tr("Booking Successful"),
                                        tr("You have successfully booked court %1 on %2 at %3.")
                                   .arg(m_padelManager->getCourtById(courtId).getName())
                                   .arg(startTime.date().toString("yyyy-MM-dd"))
                                   .arg(startTime.time().toString("HH:mm")),
                                    [this]() {
                                        handleSidebarPageChange("mybookings-section");
                                    },
                                    NotificationType::Success,
                                    7000);
        QTimer::singleShot(100, this, [this, courtId]() {
                refreshBookingsList();
                refreshTimeSlots();
            //updateCourtDetails(courtId);
            updateWaitlistStatus(courtId);
        });
    } else {
        
        if (errorMessage.contains("Court is at maximum capacity")) {
            QMessageBox::StandardButton response = QMessageBox::question(
                this,
                tr("Court At Capacity"),
                tr("The court is at maximum capacity for this time slot.\nWould you like to join the waitlist? If someone cancels, you'll be automatically notified.\n\nVIP members receive priority on the waitlist."),
                QMessageBox::Yes | QMessageBox::No
            );

            if (response == QMessageBox::Yes) {
                
                m_selectedWaitlistTime = startTime.time();
                joinWaitlist(courtId);
            }
        } else {
            NotificationManager::instance().showNotification( tr("Booking Failed"),
                                    tr("Booking failed: %1").arg(
                                    errorMessage.isEmpty() ? "Unknown error occurred" : errorMessage),
                                    nullptr,
                                    NotificationType::Error);
            QMessageBox::warning(this, tr("Booking Failed"),
                                 tr("Booking failed: %1").arg(
                                     errorMessage.isEmpty() ? "Unknown error occurred" : errorMessage));
        }

        //updateCourtDetails(courtId);
        refreshTimeSlots();
        updateWaitlistStatus(courtId);
    }
}

void BookingWindow::refreshBookingsList()
{
    m_bookingsList->clear();
    
    if (m_currentUserId <= 0) {
        m_statusLabel->setText(tr("Please log in to view your bookings."));
        return;
    }
    
    QVector<Booking> bookings = m_padelManager->getBookingsByMember(m_currentUserId);
    
    for (const Booking& booking : bookings) {
        if (!booking.isCancelled()) {
            QString bookingInfo = tr("Booking #%1: Court %2 on %3 from %4 to %5")
                                 .arg(booking.getBookingId())
                                 .arg(booking.getCourt().getName())
                                 .arg(booking.getStartTime().date().toString("yyyy-MM-dd"))
                                 .arg(booking.getStartTime().time().toString("HH:mm"))
                                 .arg(booking.getEndTime().time().toString("HH:mm"));
            
            QListWidgetItem* item = new QListWidgetItem(bookingInfo);
            item->setData(Qt::UserRole, booking.getBookingId());
            m_bookingsList->addItem(item);
        }
    }
    
    if (m_bookingsList->count() == 0) {
        m_statusLabel->setText(tr("You have no active bookings."));
    }
}

void BookingWindow::onBookingItemSelected(int row)
{
    if (row >= 0 && row < m_bookingsList->count()) {
        QListWidgetItem* idItem = m_bookingsList->item(row);
        if (idItem) {
            m_selectedBookingId = idItem->data(Qt::UserRole).toInt();
            m_cancelButton->setEnabled(true);

        Booking booking;
        for (const Booking& b : m_padelManager->getAllBookings()) {
            if (b.getBookingId() == m_selectedBookingId) {
                booking = b;
                break;
            }
        }

            bool canReschedule = canCancelOrReschedule(booking);
            m_rescheduleButton->setEnabled(canReschedule);
            m_rescheduleTimeSelector->setEnabled(canReschedule);
            
            if (canReschedule) {
                
            m_rescheduleTimeSelector->clear();
            int courtId = booking.getCourtId();
            QDate bookingDate = booking.getStartTime().date();
            
                QJsonObject courtDetails = m_padelManager->getCourtDetails(courtId);
                int maxAttendees = 2; 
                if (courtDetails.contains("maxAttendees")) {
                    maxAttendees = courtDetails["maxAttendees"].toInt();
                }
                
                if (maxAttendees <= 0) {
                    maxAttendees = 2;
                }
                
                QJsonArray availableSlotsJson = m_padelManager->getAvailableTimeSlots(courtId, bookingDate, maxAttendees);
                
                for (int i = 0; i < availableSlotsJson.size(); i++) {
                    QJsonObject slotObject = availableSlotsJson[i].toObject();
                    QString startTimeStr = slotObject["startTime"].toString();
                    QTime startTime = QTime::fromString(startTimeStr, "HH:mm");
                    
                    if (startTime.isValid()) {
                        m_rescheduleTimeSelector->addItem(startTime.toString("HH:mm"), startTime);
                    }
                }
            }
        }
    } else {
        m_selectedBookingId = -1;
        m_cancelButton->setEnabled(false);
        m_rescheduleButton->setEnabled(false);
        m_rescheduleTimeSelector->setEnabled(false);
    }
}

bool BookingWindow::canCancelOrReschedule(const Booking& booking)
{
    QDateTime now = timeLogicInstance.getCurrentTime();
    return booking.getStartTime() > now.addSecs(3 * 3600);  
}

void BookingWindow::cancelBooking() {
    qDebug() << "=== Cancel Booking Debug ===";
    qDebug() << "Selected Booking ID:" << m_selectedBookingId;
    qDebug() << "Current User ID:" << m_currentUserId;
    
    if (m_selectedBookingId < 0) {
        qDebug() << "No booking selected (ID < 0)";
        NotificationManager::instance().showNotification(tr("Warning"),
            tr("Please select a booking to cancel."),
                    nullptr,
                    NotificationType::Info);
        return;
    }
    
    QListWidgetItem* item = m_bookingsList->currentItem();
    qDebug() << "Current list item:" << (item ? "valid" : "null");
    QString bookingDetails = item ? item->text() : QString("Booking ID: %1").arg(m_selectedBookingId);
    qDebug() << "Booking details:" << bookingDetails;
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Cancel Booking"),
                                 tr("Are you sure you want to cancel this booking?\n%1").arg(bookingDetails),
                                 QMessageBox::Yes | QMessageBox::No);
    
    qDebug() << "User reply:" << (reply == QMessageBox::Yes ? "Yes" : "No");
    
    if (reply == QMessageBox::Yes) {
        qDebug() << "User" << m_currentUserEmail << "attempting to cancel booking ID:" << m_selectedBookingId;
        
        QString errorMessage;
        bool success = false;
        try {
            success = m_padelManager->cancelBooking(m_selectedBookingId, errorMessage);
            qDebug() << "Cancel booking result:" << (success ? "Success" : "Failed");
            if (!success) {
                qDebug() << "Error message:" << errorMessage;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception during cancelBooking:" << e.what();
            success = false;
            errorMessage = QString("Exception: %1").arg(e.what());
        } catch (...) {
            qDebug() << "Unknown exception during cancelBooking";
            success = false;
            errorMessage = "Unknown error occurred";
        }

        if (success) {
            NotificationManager::instance().showNotification(tr("Success"), tr("Booking cancelled successfully."),
                                    nullptr,
                                    NotificationType::Success,
                                    7000);

            qDebug() << "Refreshing booking list and time slots...";
            refreshBookingsList();
            refreshTimeSlots();

            QTimer::singleShot(1000, this, &BookingWindow::refreshTimeSlots);
        } else {
            NotificationManager::instance().showNotification(tr("Error"), tr("Failed to cancel booking: %1").arg(errorMessage),
                                    nullptr,
                                    NotificationType::Error,
                                    7000);
        }
    }
    qDebug() << "=== End Cancel Booking Debug ===";
}

void BookingWindow::rescheduleBooking() {
    
    if (m_selectedBookingId < 0) {
        NotificationManager::instance().showNotification(tr("Warning"),
            tr("Please select a booking to reschedule."),
                    nullptr,
                    NotificationType::Info);
        // QMessageBox::warning(this, tr("Warning"), tr("Please select a booking to reschedule."));
        return;
    }

    QTime newTime;
    
    if (m_rescheduleTimeSelector->metaObject()->className() == QString("QTimeEdit")) {
        QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(m_rescheduleTimeSelector);
        if (timeEdit) {
            newTime = timeEdit->time();
        }
    } else {
        int currentIndex = m_rescheduleTimeSelector->currentIndex();
        if (currentIndex >= 0) {
            QVariant timeData = m_rescheduleTimeSelector->itemData(currentIndex);
            if (timeData.canConvert<QTime>()) {
                newTime = timeData.value<QTime>();
            } else {
                QString timeText = m_rescheduleTimeSelector->currentText();
                newTime = QTime::fromString(timeText, "HH:mm");
            }
        }
    }
    
    if (!newTime.isValid()) {
        NotificationManager::instance().showNotification(tr("Warning"),
            tr("Please select a valid time."),
                    nullptr,
                    NotificationType::Info);
        // QMessageBox::warning(this, tr("Warning"), tr("Please select a valid time."));
        return;
    }

    QListWidgetItem* item = m_bookingsList->currentItem();
    if (!item) {
        NotificationManager::instance().showNotification(tr("Warning"),
                    tr("Selected booking information is not available."),
                    nullptr,
                    NotificationType::Info);
        // QMessageBox::warning(this, tr("Warning"), tr("Selected booking information is not available."));
        return;
    }

    QString bookingText = item->text();
    QDate bookingDate = timeLogicInstance.getCurrentTime().date();
    
    QRegularExpression dateRegex("(\\d{4}-\\d{2}-\\d{2})");
    QRegularExpressionMatch match = dateRegex.match(bookingText);
    if (match.hasMatch()) {
        QString dateStr = match.captured(1);
        bookingDate = QDate::fromString(dateStr, "yyyy-MM-dd");
    }
    
    if (!bookingDate.isValid()) {
        NotificationManager::instance().showNotification(tr("Warning"),
                    tr("Could not determine booking date."),
                    nullptr,
                    NotificationType::Info);
        // QMessageBox::warning(this, tr("Warning"), tr("Could not determine booking date."));
        return;
    }

    QDateTime newStartTime(bookingDate, newTime);
    QDateTime newEndTime(bookingDate, getEndTime(newTime));

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Reschedule Booking"),
                                 tr("Are you sure you want to reschedule this booking to %1?")
                                   .arg(newStartTime.toString("yyyy-MM-dd HH:mm")),
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        
        qDebug() << "User" << m_currentUserEmail << "attempting to reschedule booking ID:" 
                 << m_selectedBookingId << "to" << newStartTime.toString();
        
        QString errorMessage;
        if (m_padelManager->rescheduleBooking(m_selectedBookingId, newStartTime, newEndTime, errorMessage)) {
            NotificationManager::instance().showNotification(tr("Success"),
                    tr("Booking rescheduled successfully."),
                    nullptr,
                    NotificationType::Success);
            // QMessageBox::information(this, tr("Success"), tr("Booking rescheduled successfully."));

            refreshBookingsList();
            refreshTimeSlots();

            QTimer::singleShot(1000, this, &BookingWindow::refreshTimeSlots);
        } else {
            NotificationManager::instance().showNotification(tr("Error"),
                    tr("Failed to reschedule booking: %1").arg(errorMessage),
                    nullptr,
                    NotificationType::Error);
            // QMessageBox::critical(this, tr("Error"), tr("Failed to reschedule booking: %1").arg(errorMessage));
        }
    }
}

QLayout* BookingWindow::setupWaitlistUI(QWidget* parent) 
{
    QVBoxLayout* waitlistLayout = new QVBoxLayout();

    m_waitlistStatusLabel = new QLabel(parent);
    m_waitlistStatusLabel->setWordWrap(true);
    m_waitlistStatusLabel->setStyleSheet(waitlistStatusLabelStyle);
    waitlistLayout->addWidget(m_waitlistStatusLabel);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_joinWaitlistButton = new QPushButton(tr("Global Waitlist"), parent);
    m_joinWaitlistButton->setStyleSheet("background-color: #6B7280; color: white; border-radius: 4px; padding: 8px;");
    m_joinWaitlistButton->setEnabled(false);
    m_joinWaitlistButton->setVisible(false);
    
    m_checkWaitlistButton = new QPushButton(tr("Check Waitlist Status"), parent);
    m_checkWaitlistButton->setStyleSheet("background-color: #6B7280; color: white; border-radius: 4px; padding: 8px;");
    
    buttonLayout->addWidget(m_joinWaitlistButton);
    buttonLayout->addWidget(m_checkWaitlistButton);
    
    waitlistLayout->addLayout(buttonLayout);
    
    return waitlistLayout;
}

QTime BookingWindow::getEndTime(const QTime& startTime)
{
    return startTime.addSecs(3600);  
}

void BookingWindow::updateTheme(bool isDark)
{
    m_isDarkTheme = isDark;

    // Update corner label
    if (QLabel* cornerLabel = qobject_cast<QLabel*>(m_calendarGrid->itemAtPosition(0, 0)->widget())){
        cornerLabel->setStyleSheet(calendarCornerLabelStyle.arg(m_isDarkTheme ? "black" : "white"));
    }

    // Update calendar time headers
    for (int col = 0; col < 15; col++) {
        if (QLabel* timeLabel = qobject_cast<QLabel*>(m_calendarGrid->itemAtPosition(0, col + 1)->widget())) {
            timeLabel->setStyleSheet(calendarTimeLabelStyle.arg(m_isDarkTheme ? "#9CAFF0" : "#33498F", m_isDarkTheme ? "black" : "white"));
        }
    }
    // Update calendar day headers
    for (int row = 0; row < 7; row++) {
        if (m_dayHeaders[row]) {
            m_dayHeaders[row]->setStyleSheet(calendarDayLabelStyle.arg(m_isDarkTheme ? "#9CAFF0" : "#33498F", m_isDarkTheme ? "black" : "white"));
        }
    }

    QString lightBg = "#DCEAFF";
    QString lightText = "#111827";
    QString lightSecondaryText = "#4B5563";
    QString lightBorder = "#E5E7EB";
    QString lightHighlight = "#F3F4F6";
    
    QString darkBg = "#1F2937";
    QString darkText = "#FFFFFF";
    QString darkSecondaryText = "#9CA3AF";
    QString darkBorder = "#374151";
    QString darkHighlight = "#374151";

    QString primaryColor = "#4F46E5"; 
    QString secondaryColor = "#6B7280"; 
    QString warningColor = "#F97316"; 
    QString dangerColor = "#EF4444"; 
    QString successColor = "#10B981"; 

    QString backgroundColor = isDark ? darkBg : lightBg;
    QString textColor = isDark ? darkText : lightText;
    QString secondaryTextColor = isDark ? darkSecondaryText : lightSecondaryText;
    QString borderColor = isDark ? darkBorder : lightBorder;
    QString highlightColor = isDark ? darkHighlight : lightHighlight;

    // Update bookingsContainer style
    QWidget* bookingsContainer = findChild<QWidget*>("bookingsContainer");
    if (bookingsContainer) {
        bookingsContainer->setStyleSheet(QString(R"(
            QWidget#bookingsContainer {
                background: rgba(%1, 0.6);
                border-radius: 16px;
                border: 1px solid rgba(139, 92, 246, 0.2);
            }
        )").arg(isDark ? "0, 0, 0" : "255, 255, 255"));
    }

    // Update bookingsList style
    if (m_bookingsList) {
        m_bookingsList->setStyleSheet(bookingsListStyle.arg(isDark ? "#FFFFFF" : "#111827",isDark ? "255, 255, 255" : "0, 0, 0"));
    }

    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; }"
        "QLabel { color: %2; }"
        "QComboBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 5px; padding: 5px; }"
        "QComboBox:disabled { background-color: %5; color: %4; }"
        "QDateEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 5px; padding: 5px; }"
        "QPushButton:disabled { background-color: %5; color: %4; }"
        "QListWidget { background-color: %1; color: %2; border: 1px solid %3; border-radius: 5px; }"
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 5px; padding: 5px; }"
        "QCheckBox { color: %2; }"
        "#titleLabel { font-size: 24px; font-weight: bold; color: %6; }"
        "#sectionHeading { font-size: 18px; font-weight: bold; color: %6; }"
        "#subHeading { font-weight: bold; margin-top: 10px; color: %6; }"
        "#labelHeading { font-weight: bold; }"
        "#statusLabel, #messageLabel { color: %4; font-style: italic; }"
        "#detailLabel { padding: 5px; margin-bottom: 5px; color: %2; }"
        "#capacityLabel { font-weight: bold; color: %6; }"
    ).arg(backgroundColor, textColor, borderColor, secondaryTextColor, highlightColor, primaryColor));

    if (m_searchButton) {
        m_searchButton->setStyleSheet(helperButtonsStyle.arg(primaryColor));
    }
    
    if (m_clearSearchButton) {
        m_clearSearchButton->setStyleSheet(helperButtonsStyle.arg(secondaryColor));
    }
    
    if (m_cancelButton) {
        m_cancelButton->setStyleSheet(helperButtonsStyle.arg(dangerColor));
    }
    
    if (m_rescheduleButton) {
        m_rescheduleButton->setStyleSheet(helperButtonsStyle.arg(warningColor));
    }
    
    if (m_checkWaitlistButton) {
        m_checkWaitlistButton->setStyleSheet(helperButtonsStyle.arg(secondaryColor));
    }
    
    refreshTimeSlots();
}

void BookingWindow::retranslateUI()
{
    if (m_cancelButton) m_cancelButton->setText(tr("Cancel Selected Booking"));
    if (m_rescheduleButton) m_rescheduleButton->setText(tr("Reschedule Selected Booking"));
    if (m_searchButton) m_searchButton->setText(tr("Search"));
    if (m_clearSearchButton) m_clearSearchButton->setText(tr("Clear"));
    if (m_joinWaitlistButton) m_joinWaitlistButton->setText(tr("Global Waitlist"));
    if (m_checkWaitlistButton) m_checkWaitlistButton->setText(tr("Check Waitlist Status"));

    refreshBookingsList();
}

void BookingWindow::setupSearchUI(QWidget* parent, QVBoxLayout* parentLayout)
{
    QVBoxLayout* searchLayout = new QVBoxLayout();

    QLabel* searchTitle = new QLabel(tr("Search & Filter Courts"), parent);
    searchTitle->setStyleSheet(searchTitleStyle);
    searchLayout->addWidget(searchTitle);

    QHBoxLayout* searchControlsLayout = new QHBoxLayout();

    m_searchBox = new QLineEdit(parent);
    m_searchBox->setPlaceholderText(tr("Search by court name..."));
    m_searchBox->setClearButtonEnabled(true);
    m_searchBox->setStyleSheet(searchBoxStyle);

    m_locationFilter = new QComboBox(parent);
    m_locationFilter->addItem(tr("All Locations"), "");
    m_locationFilter->setStyleSheet(locationFilterStyle);

    m_showAvailableOnly = new QCheckBox("", parent);
    m_showAvailableOnly->setVisible(false);

    m_searchButton = new QPushButton(tr("Search"), parent);
    m_searchButton->setStyleSheet(searchButtonStyle);
    
    m_clearSearchButton = new QPushButton(tr("Clear"), parent);
    m_clearSearchButton->setStyleSheet(clearSearchButtonStyle);

    searchControlsLayout->addWidget(m_searchBox, 4);
    searchControlsLayout->addWidget(m_locationFilter, 3);
    searchControlsLayout->addWidget(m_searchButton, 1);
    searchControlsLayout->addWidget(m_clearSearchButton, 1);
    
    searchLayout->addLayout(searchControlsLayout);

    m_totalResultsLabel = new QLabel(parent);
    m_totalResultsLabel->setStyleSheet(totalResultsLabelStyle);
    searchLayout->addWidget(m_totalResultsLabel);

    parentLayout->addLayout(searchLayout);
}

void BookingWindow::bookTimeSlot(int courtId, const QDate& date, const QString& startTimeStr, const QString& endTimeStr)
{
    try {
        qDebug() << "Booking time slot for court" << courtId << "on" << date.toString()
                << "from" << startTimeStr << "to" << endTimeStr;

        if (m_currentUserId <= 0) {
            qDebug() << "Cannot book time slot: No user is logged in";
            QMessageBox::warning(this, tr("Login Required"), 
                                tr("You must be logged in to book a time slot."));
            return;
        }

        QTime startTime = QTime::fromString(startTimeStr, "HH:mm");
        QTime endTime = QTime::fromString(endTimeStr, "HH:mm");
        
        if (!startTime.isValid() || !endTime.isValid()) {
            qDebug() << "ERROR: Invalid time format for booking:" << startTimeStr << "-" << endTimeStr;
            QMessageBox::warning(this, tr("Booking Error"), 
                                tr("Invalid time format. Please try again."));
            return;
        }

        QDateTime startDateTime(date, startTime);
        QDateTime endDateTime(date, endTime);

        if (!m_padelManager->isCourtAvailable(courtId, startDateTime, endDateTime)) {
            bookCourtDirectly(courtId, startDateTime, endDateTime);
            return;
        }

        QString message = tr("Do you want to book court %1 on %2 from %3 to %4?")
                         .arg(m_courtSelector->currentText())
                         .arg(date.toString("dd/MM/yyyy"))
                         .arg(startTimeStr)
                         .arg(endTimeStr);
        
        int result = QMessageBox::question(this, tr("Confirm Booking"), message, 
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (result == QMessageBox::Yes) {
            bookCourtDirectly(courtId, startDateTime, endDateTime);
        }
    } catch (const std::exception& e) {
        qDebug() << "ERROR: Exception in bookTimeSlot:" << e.what();
        QMessageBox::warning(this, tr("Booking Error"), 
                            tr("An error occurred while booking: %1").arg(e.what()));
    } catch (...) {
        qDebug() << "ERROR: Unknown exception in bookTimeSlot";
        QMessageBox::warning(this, tr("Booking Error"), 
                            tr("An unknown error occurred while booking."));
    }
}

QWidget* BookingWindow::setupAlternativeCourtsUI(QWidget* parent)
{
    m_alternativeCourtsWidget = new QWidget(parent);
    m_alternativeCourtsLayout = new QVBoxLayout(m_alternativeCourtsWidget);
    m_alternativeCourtsLayout->setContentsMargins(0, 10, 0, 10);
    
    m_alternativesLabel = new QLabel(tr("Alternative Courts with Available Times:"), m_alternativeCourtsWidget);
    m_alternativesLabel->setStyleSheet(alternativesLabelStyle);
    m_alternativeCourtsLayout->addWidget(m_alternativesLabel);
    
    QWidget* alternativesContainer = new QWidget(m_alternativeCourtsWidget);
    m_alternativeCourtsList = new QGridLayout(alternativesContainer);
    m_alternativeCourtsList->setSpacing(10);
    m_alternativeCourtsList->setContentsMargins(0, 10, 0, 10);
    
    m_alternativeCourtsLayout->addWidget(alternativesContainer);
    
    return m_alternativeCourtsWidget;
}

void BookingWindow::updateAlternativeCourts() 
{
    try {
        if (!m_padelManager || !m_alternativeCourtsList || !m_courtSelector || !m_dateSelector) {
            qDebug() << "ERROR: null components in updateAlternativeCourts";
            return;
        }
        
        QLayoutItem* item;
        while ((item = m_alternativeCourtsList->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        
        int currentCourtId = m_courtSelector->currentData().toInt();
        if (currentCourtId <= 0) {
            return;
        }
        
        QDate selectedDate = m_dateSelector->date();
        if (!selectedDate.isValid()) {
            selectedDate = timeLogicInstance.getCurrentTime().date();
        }
        
        QTime defaultTime(12, 0);
        QDateTime defaultDateTime(selectedDate, defaultTime);
        
        QString locationFilter = "";
        if (m_locationFilter && m_locationFilter->currentIndex() > 0) {
            locationFilter = m_locationFilter->currentData().toString();
        }
        
        QVector<Court> allCourts = m_padelManager->getAllCourts();
        if (allCourts.isEmpty()) {
            m_alternativeCourtsWidget->setVisible(true);
            QLabel* noAlternativesLabel = new QLabel(tr("No alternative courts with available times"));
            noAlternativesLabel->setAlignment(Qt::AlignCenter);
            noAlternativesLabel->setStyleSheet(noAlternativesLabelStyle);
            m_alternativeCourtsList->addWidget(noAlternativesLabel, 0, 0);
            return;
        }
        
        int row = 0;
        int col = 0;
        int maxCols = 3;
        int count = 0;
    
        for (const Court& court : allCourts) {
            if (court.getId() == currentCourtId) {
                continue;
            }
            
            if (!locationFilter.isEmpty() && court.getLocation() != locationFilter) {
                continue;
            }
            
            QJsonObject courtDetails = m_padelManager->getCourtDetails(court.getId());
            int maxAttendees = 2; 
            if (courtDetails.contains("maxAttendees")) {
                maxAttendees = courtDetails["maxAttendees"].toInt();
            }
            
            QJsonArray availableTimeSlots = m_padelManager->getAvailableTimeSlots(court.getId(), selectedDate, maxAttendees);
            
            if (!availableTimeSlots.isEmpty()) {
                QFrame* courtFrame = new QFrame();
                courtFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
                courtFrame->setLineWidth(2);
                courtFrame->setMidLineWidth(1);
                courtFrame->setFixedSize(250, 180);
                courtFrame->setStyleSheet(courtFrameStyle);
                
                QVBoxLayout* courtLayout = new QVBoxLayout(courtFrame);
                courtLayout->setSpacing(8);
                courtLayout->setContentsMargins(12, 12, 12, 12);
                
                QString courtName = court.getName();
                QString location = court.getLocation();
                
                QLabel* nameLabel = new QLabel(courtName);
                nameLabel->setStyleSheet(nameLabelStyle);
                courtLayout->addWidget(nameLabel);
                
                QLabel* locationLabel = new QLabel(location);
                locationLabel->setStyleSheet(locationLabelStyle);
                courtLayout->addWidget(locationLabel);
                
                QFrame* line = new QFrame();
                line->setFrameShape(QFrame::HLine);
                line->setFrameShadow(QFrame::Sunken);
                line->setStyleSheet("background-color: black;");
                line->setMinimumHeight(2);
                courtLayout->addWidget(line);
                
                QLabel* timesLabel = new QLabel(tr("Available Times:"));
                timesLabel->setStyleSheet(timesLabelStyle);
                courtLayout->addWidget(timesLabel);
                
                QVBoxLayout* timesLayout = new QVBoxLayout();
                timesLayout->setSpacing(5);
                
                int displayLimit = qMin(3, availableTimeSlots.size());
                for (int i = 0; i < displayLimit; i++) {
                    QJsonObject slot = availableTimeSlots[i].toObject();
                    QString startTime = slot["startTime"].toString();
                    QString endTime = slot["endTime"].toString();
                    int available = slot["availableSpots"].toInt();
                    
                    QString timeText = QString("%1-%2 (%3 spots)").arg(startTime).arg(endTime).arg(available);
                    
                    QLabel* timeLabel = new QLabel(timeText);
                    timeLabel->setStyleSheet(timeLabelStyle);
                    timesLayout->addWidget(timeLabel);
                    
                    if (i == 0) {
                        courtFrame->setProperty("startTime", startTime);
                    }
                }
                
                courtLayout->addLayout(timesLayout);
                
                QPushButton* bookButton = new QPushButton(tr("Book"));
                bookButton->setStyleSheet(bookAltCourtButtonStyle);
                bookButton->setCursor(Qt::PointingHandCursor);
                bookButton->setMinimumHeight(30);  
                courtLayout->addWidget(bookButton);
                
                courtFrame->setProperty("courtId", court.getId());
                
                connect(bookButton, &QPushButton::clicked, this, [this, court, courtFrame]() {
                    int courtId = courtFrame->property("courtId").toInt();
                    QString startTimeStr = courtFrame->property("startTime").toString();
                    onAlternativeCourtButtonClicked(courtId, startTimeStr);
                });
                
                m_alternativeCourtsList->addWidget(courtFrame, row, col);
                
                col++;
                if (col >= maxCols) {
                    col = 0;
                    row++;
                }
                count++;
            }
        }
        
        m_alternativeCourtsWidget->setVisible(true);
        
        if (count == 0) {
            QLabel* noAlternativesLabel = new QLabel(tr("No alternative courts with available times"));
            noAlternativesLabel->setAlignment(Qt::AlignCenter);
            noAlternativesLabel->setStyleSheet(noAlternativesLabelStyle);
            m_alternativeCourtsList->addWidget(noAlternativesLabel, 0, 0);
        }
    }
    catch (const std::exception& e) {
        qDebug() << "ERROR in updateAlternativeCourts:" << e.what();
    }
    catch (...) {
        qDebug() << "UNKNOWN ERROR in updateAlternativeCourts";
    }
}

void BookingWindow::onAlternativeCourtButtonClicked(int courtId, const QString& timeSlot)
{
    try {
        if (courtId <= 0 || timeSlot.isEmpty()) {
            return;
        }
        
        for (int i = 0; i < m_courtSelector->count(); i++) {
            if (m_courtSelector->itemData(i).toInt() == courtId) {
                m_courtSelector->setCurrentIndex(i);
                break;
            }
        }
        
        QDate selectedDate = m_dateSelector->date();
        QTime startTime = QTime::fromString(timeSlot, "HH:mm");
        
        if (selectedDate.isValid() && startTime.isValid()) {
            QDateTime startDateTime(selectedDate, startTime);
            QDateTime endDateTime = startDateTime.addSecs(3600);
            
            QString courtName = m_courtSelector->currentText();
            QString courtLocation = "";
            
            QJsonObject courtDetails = m_padelManager->getCourtDetails(courtId);
            if (courtDetails.contains("location")) {
                courtLocation = courtDetails["location"].toString();
            }
            
            QMessageBox msgBox(this);
            msgBox.setWindowTitle(tr("Book Alternative Court"));
            msgBox.setIconPixmap(QPixmap(":/icons/booking.png").scaled(QSize(48, 48)));
            
            QString message = tr("<h3>Book this court?</h3>") +
                          tr("<p><b>Court:</b> %1</p>").arg(courtName) +
                          tr("<p><b>Location:</b> %1</p>").arg(courtLocation) +
                          tr("<p><b>Date:</b> %1</p>").arg(selectedDate.toString("dd/MM/yyyy")) +
                          tr("<p><b>Time:</b> %1 - %2</p>").arg(startTime.toString("HH:mm"))
                                                        .arg(endDateTime.time().toString("HH:mm"));
            
            msgBox.setText(message);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            
            int result = msgBox.exec();
            
            if (result == QMessageBox::Yes) {
                bookCourtDirectly(courtId, startDateTime, endDateTime);
            }
        }
    }
    catch (const std::exception& e) {}
    catch (...) {}
}
