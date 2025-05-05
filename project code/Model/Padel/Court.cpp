#include "Court.h"
#include <QDebug>

// Default constructor
Court::Court()
    : m_courtId(0)
    , m_name("")
    , m_location("")
    , m_isIndoor(false)
    , m_pricePerHour(0.0)
    , m_description("")
    , m_maxAttendees(4)
{
    // Initialize with default time slots
    initializeDefaultTimeSlots();
}

// Parameterized constructor
Court::Court(int courtId, const QString& name, const QString& location, bool isIndoor, double pricePerHour)
    : m_courtId(courtId)
    , m_name(name)
    , m_location(location)
    , m_isIndoor(isIndoor)
    , m_pricePerHour(pricePerHour)
    , m_description("")
    , m_maxAttendees(4)
{
    initializeDefaultTimeSlots();
}

// Helper to add default time slots if none exist
void Court::initializeDefaultTimeSlots() {
    if (m_timeSlots.empty()) {
        for (int hour = 9; hour <= 21; hour++) {
            m_timeSlots.push_back(QTime(hour, 0));
        }
        
        qDebug() << "Initialized court" << m_courtId << "with" << m_timeSlots.size() << "default time slots";
    }
}

// Availability management
bool Court::isAvailable(const QDateTime& time) const {
    return m_availability.value(time, true);
}

void Court::setTimeSlot(const QDateTime& time, bool available) {
    m_availability[time] = available;
}

// Safe accessor for time slots
const std::vector<QTime>& Court::getSafeTimeSlots() const {
    if (m_timeSlots.empty() || m_timeSlots.size() > 100) {
        static std::vector<QTime> defaultSlots;
        if (defaultSlots.empty()) {
            for (int hour = 9; hour <= 21; hour++) {
                defaultSlots.push_back(QTime(hour, 0));
            }
        }
        return defaultSlots;
    }
    return m_timeSlots;
} 