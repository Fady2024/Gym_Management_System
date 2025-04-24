#include "Court.h"

// Default constructor
Court::Court()
    : m_courtId(0)
    , m_name("")
    , m_location("")
    , m_isIndoor(false)
    , m_pricePerHour(0.0)
{
}

// Parameterized constructor
Court::Court(int courtId, const QString& name, const QString& location, bool isIndoor, double pricePerHour)
    : m_courtId(courtId)
    , m_name(name)
    , m_location(location)
    , m_isIndoor(isIndoor)
    , m_pricePerHour(pricePerHour)
{
}

// Availability management
bool Court::isAvailable(const QDateTime& time) const {
    return m_availability.value(time, true);
}

void Court::setTimeSlot(const QDateTime& time, bool available) {
    m_availability[time] = available;
} 