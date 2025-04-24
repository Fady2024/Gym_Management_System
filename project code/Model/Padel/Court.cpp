#include "Court.h"

// Default constructor
Court::Court()
    : m_id(0), m_name(""), m_location("") {}

// Parameterized constructor
Court::Court(int id, const QString& name, const QString& location)
    : m_id(id), m_name(name), m_location(location) {}

// Getters
int Court::getId() const {
    return m_id;
}

QString Court::getName() const {
    return m_name;
}

QString Court::getLocation() const {
    return m_location;
}

const vector<QTime>& Court::getAllTimeSlots() const {
    return allTimeSlots;
}

// Setters
void Court::setId(int id) {
    m_id = id;
}

void Court::setName(const QString& name) {
    m_name = name;
}

void Court::setLocation(const QString& location) {
    m_location = location;
}

