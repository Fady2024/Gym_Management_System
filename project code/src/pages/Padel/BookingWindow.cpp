#include "BookingWindow.h"
#include <QDebug>
#include <QApplication>
#include <QTimer>

BookingWindow::BookingWindow(PadelDataManager* padelManager, QWidget* parent)
    : QWidget(parent), m_padelManager(padelManager), m_currentUserId(0)
{
    setupUI();
    setupConnections();
    loadCourts();
}

void BookingWindow::setCurrentUserEmail(const QString& email)
{
    m_currentUserEmail = email;
    loadUserData();
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

    // Get user data from email
    User user = m_userDataManager->getUserData(m_currentUserEmail);
    if (user.getId() <= 0) {
        qDebug() << "Error: Invalid user ID for email" << m_currentUserEmail;
        return;
    }

    m_currentUserId = user.getId();
    qDebug() << "Loaded user ID" << m_currentUserId << "from email" << m_currentUserEmail;

    // Check if user is a member
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

    // Update UI with user info
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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // Title
    QLabel* titleLabel = new QLabel(tr("Padel Court Booking System"), this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    
    // User info label
    m_userInfoLabel = new QLabel(tr("User: Not logged in"), this);
    m_userInfoLabel->setStyleSheet("font-size: 14px; color: #666;");
    mainLayout->addWidget(m_userInfoLabel);

    mainLayout->addWidget(createSeparator());

    // Court selection section
    QHBoxLayout* courtSelectionLayout = new QHBoxLayout();
    mainLayout->addLayout(courtSelectionLayout);

    // Left panel - Court selection
    QVBoxLayout* selectionLayout = new QVBoxLayout();
    selectionLayout->setSpacing(10);
    courtSelectionLayout->addLayout(selectionLayout, 3);

    // Court selector
    QHBoxLayout* courtLayout = new QHBoxLayout();
    QLabel* courtLabel = new QLabel(tr("Select Court:"), this);
    m_courtSelector = new QComboBox(this);
    m_courtSelector->setMinimumWidth(200);
    courtLayout->addWidget(courtLabel);
    courtLayout->addWidget(m_courtSelector);
    selectionLayout->addLayout(courtLayout);

    // Date selector
    QHBoxLayout* dateLayout = new QHBoxLayout();
    QLabel* dateLabel = new QLabel(tr("Select Date:"), this);
    m_dateSelector = new QDateEdit(QDate::currentDate(), this);
    m_dateSelector->setCalendarPopup(true);
    m_dateSelector->setMinimumDate(QDate::currentDate());
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(m_dateSelector);
    selectionLayout->addLayout(dateLayout);

    // Time slot selector
    QHBoxLayout* timeLayout = new QHBoxLayout();
    QLabel* timeLabel = new QLabel(tr("Select Time:"), this);
    m_timeSlotSelector = new QComboBox(this);
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(m_timeSlotSelector);
    selectionLayout->addLayout(timeLayout);

    // Book button
    m_bookButton = new QPushButton(tr("Book Court"), this);
    m_bookButton->setFixedHeight(40);
    m_bookButton->setStyleSheet("background-color: #8B5CF6; color: white; font-weight: bold; border-radius: 5px;");
    selectionLayout->addWidget(m_bookButton);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    selectionLayout->addWidget(m_statusLabel);
    
    // Spacer
    selectionLayout->addStretch();

    // Right panel - Court details
    QVBoxLayout* detailsLayout = new QVBoxLayout();
    courtSelectionLayout->addLayout(detailsLayout, 2);

    QLabel* detailsTitle = new QLabel(tr("Court Details"), this);
    detailsTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    detailsLayout->addWidget(detailsTitle);

    m_courtNameLabel = new QLabel(this);
    m_courtLocationLabel = new QLabel(this);
    m_courtPriceLabel = new QLabel(this);
    m_courtDescriptionLabel = new QLabel(this);
    m_courtDescriptionLabel->setWordWrap(true);
    
    detailsLayout->addWidget(m_courtNameLabel);
    detailsLayout->addWidget(m_courtLocationLabel);
    detailsLayout->addWidget(m_courtPriceLabel);
    
    detailsLayout->addWidget(createSeparator());
    
    detailsLayout->addWidget(m_courtDescriptionLabel);
    
    QLabel* featuresTitle = new QLabel(tr("Features:"), this);
    featuresTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");
    detailsLayout->addWidget(featuresTitle);
    
    m_courtFeaturesListWidget = new QListWidget(this);
    m_courtFeaturesListWidget->setMaximumHeight(120);
    m_courtFeaturesListWidget->setStyleSheet("background-color: transparent; border: none;");
    detailsLayout->addWidget(m_courtFeaturesListWidget);
    
    detailsLayout->addStretch();

    mainLayout->addWidget(createSeparator());

    // Bookings section
    QLabel* bookingsTitle = new QLabel(tr("My Bookings"), this);
    bookingsTitle->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(bookingsTitle);

    m_bookingsList = new QListWidget(this);
    m_bookingsList->setMinimumHeight(150);
    mainLayout->addWidget(m_bookingsList);

    // Booking management section
    QHBoxLayout* bookingManagementLayout = new QHBoxLayout();
    mainLayout->addLayout(bookingManagementLayout);

    m_cancelButton = new QPushButton(tr("Cancel Selected Booking"), this);
    m_cancelButton->setEnabled(false);
    bookingManagementLayout->addWidget(m_cancelButton);

    m_rescheduleButton = new QPushButton(tr("Reschedule Selected Booking"), this);
    m_rescheduleButton->setEnabled(false);
    bookingManagementLayout->addWidget(m_rescheduleButton);

    m_rescheduleTimeSelector = new QComboBox(this);
    m_rescheduleTimeSelector->setEnabled(false);
    bookingManagementLayout->addWidget(m_rescheduleTimeSelector);
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
    connect(m_bookButton, &QPushButton::clicked, this, &BookingWindow::bookCourt);
    connect(m_bookingsList, &QListWidget::currentRowChanged, this, &BookingWindow::onBookingItemSelected);
    connect(m_cancelButton, &QPushButton::clicked, this, &BookingWindow::cancelBooking);
    connect(m_rescheduleButton, &QPushButton::clicked, this, &BookingWindow::rescheduleBooking);
}

void BookingWindow::loadCourts()
{
    m_courtSelector->clear();
    
    QVector<Court> courts = m_padelManager->getAllCourts();
    for (const Court& court : courts) {
        m_courtSelector->addItem(court.getName(), court.getId());
    }
    
    if (m_courtSelector->count() > 0) {
        onCourtSelectionChanged(0);
    }
}

void BookingWindow::onCourtSelectionChanged(int index)
{
    if (index >= 0) {
        int courtId = m_courtSelector->itemData(index).toInt();
        updateCourtDetails(courtId);
        refreshTimeSlots();
    }
}

void BookingWindow::updateCourtDetails(int courtId)
{
    Court court = m_padelManager->getCourtById(courtId);
    
    m_courtNameLabel->setText(tr("Name: %1").arg(court.getName()));
    m_courtLocationLabel->setText(tr("Location: %1").arg(court.getLocation()));
    m_courtPriceLabel->setText(tr("Price per hour: $%1").arg(QString::number(court.getPricePerHour(), 'f', 2)));
    
    // Show court description
    if (!court.getDescription().isEmpty()) {
        m_courtDescriptionLabel->setText(court.getDescription());
        m_courtDescriptionLabel->setVisible(true);
    } else {
        m_courtDescriptionLabel->setVisible(false);
    }
    
    // Show court features
    m_courtFeaturesListWidget->clear();
    const QStringList& features = court.getFeatures();
    if (!features.isEmpty()) {
        for (const QString& feature : features) {
            QListWidgetItem* item = new QListWidgetItem("• " + feature);
            m_courtFeaturesListWidget->addItem(item);
        }
        m_courtFeaturesListWidget->setVisible(true);
    } else {
        m_courtFeaturesListWidget->setVisible(false);
    }
}

void BookingWindow::onDateChanged(const QDate& date)
{
    refreshTimeSlots();
}

void BookingWindow::refreshTimeSlots()
{
    m_timeSlotSelector->clear();
    
    if (m_courtSelector->count() > 0) {
        int courtId = m_courtSelector->currentData().toInt();
        QDate selectedDate = m_dateSelector->date();
        
        QVector<QTime> availableSlots = m_padelManager->getAvailableTimeSlots(courtId, selectedDate);
        
        // Filter out past time slots if the selected date is today
        if (selectedDate == QDate::currentDate()) {
            QTime currentTime = QTime::currentTime();
            
            // Add 30-minute buffer to current time (don't show slots starting within 30 min)
            QTime bookingCutoffTime = currentTime.addSecs(30 * 60);
            
            QVector<QTime> filteredSlots;
            for (const QTime& time : availableSlots) {
                if (time > bookingCutoffTime) {
                    filteredSlots.append(time);
                }
            }
            
            availableSlots = filteredSlots;
        }
        
        for (const QTime& time : availableSlots) {
            m_timeSlotSelector->addItem(time.toString("HH:mm"), time);
        }
        
        m_bookButton->setEnabled(m_timeSlotSelector->count() > 0);
        
        if (m_timeSlotSelector->count() == 0) {
            if (selectedDate == QDate::currentDate()) {
                m_statusLabel->setText(tr("No more available slots for today. Please select another date."));
            } else {
                m_statusLabel->setText(tr("No available slots for this court on the selected date."));
                QVector<QDateTime> suggestions = m_padelManager->suggestNextSlots(courtId, QDateTime(selectedDate, QTime(0, 0)));
                if (!suggestions.isEmpty()) {
                    QString suggestionText = tr("Suggested times: ");
                    for (int i = 0; i < qMin(3, suggestions.size()); ++i) {
                        if (i > 0) suggestionText += ", ";
                        suggestionText += suggestions[i].toString("yyyy-MM-dd HH:mm");
                    }
                    m_statusLabel->setText(m_statusLabel->text() + " " + suggestionText);
                }
            }
        } else {
            m_statusLabel->clear();
        }
    }
}

QTime BookingWindow::getEndTime(const QTime& startTime)
{
    return startTime.addSecs(3600);  // Add 1 hour
}

void BookingWindow::bookCourt()
{
    if (m_currentUserId <= 0) {
        QMessageBox::warning(this, tr("User Error"), 
                           tr("Please log in to book a court. Current user ID: %1").arg(m_currentUserId));
        return;
    }
    
    if (m_courtSelector->count() <= 0 || m_timeSlotSelector->count() <= 0) {
        QMessageBox::warning(this, tr("Selection Error"), 
                           tr("Please select a court and time slot."));
        return;
    }
    
    try {
        qDebug() << "Starting booking process...";
        
        // Get selected court and time
        int courtId = m_courtSelector->currentData().toInt();
        QDate selectedDate = m_dateSelector->date();
        QTime selectedTime;
        
        // Extract the time data safely - use direct string parsing
        QString timeStr = m_timeSlotSelector->currentText();
        selectedTime = QTime::fromString(timeStr, "HH:mm");
        qDebug() << "Time parsed from text:" << selectedTime.toString();
        
        if (!selectedTime.isValid()) {
            QMessageBox::warning(this, tr("Time Error"), 
                               tr("Selected time is invalid. Please try again."));
            return;
        }
        QDateTime startTime;
        startTime.setDate(selectedDate);
        startTime.setTime(selectedTime);
        
        // Add exactly one hour for end time
        QDateTime endTime = startTime.addSecs(3600);
        
        qDebug() << "Attempting to book court" << courtId << "for user" << m_currentUserId 
                << "at time" << startTime.toString("yyyy-MM-dd HH:mm");
        
        // Perform booking - Capture error message
        QString errorMessage;
        
        // Disable UI elements during booking
        m_bookButton->setEnabled(false);
        setCursor(Qt::WaitCursor);
        
        bool success = false;
        
        // Use try-catch for booking call
        try {
            success = m_padelManager->createBooking(m_currentUserId, courtId, startTime, endTime, errorMessage);
        }
        catch (const std::exception& e) {
            setCursor(Qt::ArrowCursor);
            m_bookButton->setEnabled(true);
            QMessageBox::critical(this, tr("Booking Error"), 
                            tr("Exception occurred: %1").arg(e.what()));
            return;
        }
        catch (...) {
            QString errorMsg = "Unknown error during booking. Please try again.";
            qDebug() << errorMsg;
            QMessageBox::critical(this, tr("Booking Error"), 
                            tr("An unknown error occurred while booking."));
            setCursor(Qt::ArrowCursor);
            m_bookButton->setEnabled(true);
            return;
        }
        
        // Restore UI
        setCursor(Qt::ArrowCursor);
        m_bookButton->setEnabled(true);
        
        if (success) {
            QMessageBox::information(this, tr("Booking Successful"), 
                                   tr("You have successfully booked court %1 on %2 at %3.")
                                   .arg(m_courtSelector->currentText())
                                   .arg(selectedDate.toString("yyyy-MM-dd"))
                                   .arg(selectedTime.toString("HH:mm")));
            
            // No need to save immediately - data will be saved when the app closes
            qDebug() << "Booking created successfully. Data will be saved on application exit.";
            
            // Refresh the views - delay slightly to allow UI to update
            QTimer::singleShot(100, this, [this]() {
                qDebug() << "Refreshing booking list...";
                refreshBookingsList();
                qDebug() << "Refreshing time slots...";
                refreshTimeSlots();
                qDebug() << "Booking process completed successfully";
            });
        } else {
            QMessageBox::warning(this, tr("Booking Failed"), 
                               tr("Booking failed: %1").arg(errorMessage.isEmpty() ? 
                                                         "Unknown error occurred" : errorMessage));
        }
    } 
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Booking Error"), 
                            tr("An exception occurred while booking: %1").arg(e.what()));
    } 
    catch (...) {
        QMessageBox::critical(this, tr("Booking Error"), 
                            tr("An unknown error occurred while booking."));
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
    if (row >= 0) {
        QListWidgetItem* item = m_bookingsList->item(row);
        m_selectedBookingId = item->data(Qt::UserRole).toInt();
        
        Booking booking;
        for (const Booking& b : m_padelManager->getAllBookings()) {
            if (b.getBookingId() == m_selectedBookingId) {
                booking = b;
                break;
            }
        }
        
        bool canModify = canCancelOrReschedule(booking);
        m_cancelButton->setEnabled(canModify);
        m_rescheduleButton->setEnabled(canModify);
        m_rescheduleTimeSelector->setEnabled(canModify);
        
        if (canModify) {
            m_rescheduleTimeSelector->clear();
            int courtId = booking.getCourtId();
            QDate bookingDate = booking.getStartTime().date();
            
            QVector<QTime> availableSlots = m_padelManager->getAvailableTimeSlots(courtId, bookingDate);
            for (const QTime& time : availableSlots) {
                m_rescheduleTimeSelector->addItem(time.toString("HH:mm"), time);
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
    QDateTime now = QDateTime::currentDateTime();
    return booking.getStartTime() > now.addSecs(3 * 3600);  // 3 hours before
}

void BookingWindow::cancelBooking()
{
    if (m_selectedBookingId < 0) return;
    
    QMessageBox::StandardButton confirmation = QMessageBox::question(
        this, tr("Confirm Cancellation"),
        tr("Are you sure you want to cancel this booking?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (confirmation == QMessageBox::Yes) {
        QString errorMessage;
        bool success = m_padelManager->cancelBooking(m_selectedBookingId, errorMessage);
        
        if (success) {
            QMessageBox::information(this, tr("Cancellation Successful"), 
                                  tr("Your booking has been cancelled."));
            refreshBookingsList();
            refreshTimeSlots();
        } else {
            QMessageBox::warning(this, tr("Cancellation Failed"), errorMessage);
        }
    }
}

void BookingWindow::rescheduleBooking()
{
    if (m_selectedBookingId < 0 || m_rescheduleTimeSelector->count() <= 0) return;
    
    QMessageBox::StandardButton confirmation = QMessageBox::question(
        this, tr("Confirm Reschedule"),
        tr("Are you sure you want to reschedule this booking?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (confirmation == QMessageBox::Yes) {
        QTime newTime = m_rescheduleTimeSelector->currentData().toTime();
        
        Booking booking;
        for (const Booking& b : m_padelManager->getAllBookings()) {
            if (b.getBookingId() == m_selectedBookingId) {
                booking = b;
                break;
            }
        }
        
        QDate bookingDate = booking.getStartTime().date();
        QDateTime newStartTime(bookingDate, newTime);
        QDateTime newEndTime(bookingDate, getEndTime(newTime));
        
        QString errorMessage;
        bool success = m_padelManager->rescheduleBooking(
            m_selectedBookingId, newStartTime, newEndTime, errorMessage);
        
        if (success) {
            QMessageBox::information(this, tr("Reschedule Successful"), 
                                  tr("Your booking has been rescheduled."));
            refreshBookingsList();
            refreshTimeSlots();
        } else {
            QMessageBox::warning(this, tr("Reschedule Failed"), errorMessage);
        }
    }
}

void BookingWindow::updateTheme(bool isDark)
{
    m_isDarkTheme = isDark;
    
    QString backgroundColor = isDark ? "#1F2937" : "#FFFFFF";
    QString textColor = isDark ? "#FFFFFF" : "#111827";
    QString secondaryTextColor = isDark ? "#9CA3AF" : "#4B5563";
    QString buttonColor = isDark ? "#8B5CF6" : "#8B5CF6";
    QString buttonTextColor = "#FFFFFF";
    
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; }"
        "QLabel { color: %2; }"
        "QComboBox { background-color: %1; color: %2; border: 1px solid #D1D5DB; border-radius: 5px; padding: 5px; }"
        "QComboBox:disabled { background-color: #F3F4F6; color: #9CA3AF; }"
        "QDateEdit { background-color: %1; color: %2; border: 1px solid #D1D5DB; border-radius: 5px; padding: 5px; }"
        "QPushButton { background-color: %3; color: %4; border-radius: 5px; padding: 8px; }"
        "QPushButton:disabled { background-color: #D1D5DB; color: #9CA3AF; }"
        "QListWidget { background-color: %1; color: %2; border: 1px solid #D1D5DB; border-radius: 5px; }"
    ).arg(backgroundColor, textColor, buttonColor, buttonTextColor));
}

void BookingWindow::retranslateUI()
{
    if (m_bookButton) m_bookButton->setText(tr("Book Court"));
    if (m_cancelButton) m_cancelButton->setText(tr("Cancel Selected Booking"));
    if (m_rescheduleButton) m_rescheduleButton->setText(tr("Reschedule Selected Booking"));
    
    refreshBookingsList();
}

