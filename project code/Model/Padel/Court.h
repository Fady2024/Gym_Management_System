#ifndef COURT_H
#define COURT_H

#include <QString>
#include <QDateTime>
#include <QMap>
#include <vector>
#include <QTime>
#include <QStringList>

class Court {
public:
    // Constructors
    Court();
    Court(int courtId, const QString& name, const QString& location, bool isIndoor, double pricePerHour);

    // Getters
    int getId() const { return m_courtId; }
    const QString& getName() const { return m_name; }
    const QString& getLocation() const { return m_location; }
    bool isIndoor() const { return m_isIndoor; }
    double getPricePerHour() const { return m_pricePerHour; }
    const QString& getDescription() const { return m_description; }
    const QStringList& getFeatures() const { return m_features; }
    const QMap<QDateTime, bool>& getAvailability() const { return m_availability; }
    
    std::vector<QTime>& getAllTimeSlots() {
        if (m_timeSlots.size() > 100) {
            m_timeSlots.clear();
            initializeDefaultTimeSlots();
        } else if (m_timeSlots.empty()) {
            initializeDefaultTimeSlots();
        }
        return m_timeSlots; 
    }
    
    const std::vector<QTime>& getAllTimeSlots() const { 
        if (m_timeSlots.size() > 100 || m_timeSlots.empty()) {
            return getSafeTimeSlots();
        }
        return m_timeSlots; 
    }
    
    const std::vector<QTime>& getSafeTimeSlots() const;
    
    int getMaxAttendees() const { return m_maxAttendees > 0 ? m_maxAttendees : 4; }

    // Setters
    void setId(int id) { m_courtId = id; }
    void setName(const QString& name) { m_name = name; }
    void setLocation(const QString& location) { m_location = location; }
    void setIndoor(bool indoor) { m_isIndoor = indoor; }
    void setPricePerHour(double price) { m_pricePerHour = price; }
    void setDescription(const QString& description) { m_description = description; }
    void setFeatures(const QStringList& features) { m_features = features; }
    void setAvailability(const QMap<QDateTime, bool>& availability) { m_availability = availability; }
    void setMaxAttendees(int max) { m_maxAttendees = max; }

    // Availability management
    bool isAvailable(const QDateTime& time) const;
    void setTimeSlot(const QDateTime& time, bool available);

    // Helper for initializing default time slots
    void initializeDefaultTimeSlots();

private:
    int m_courtId;
    QString m_name;
    QString m_location;
    bool m_isIndoor;
    double m_pricePerHour;
    QString m_description;
    QStringList m_features;
    QMap<QDateTime, bool> m_availability;
    std::vector<QTime> m_timeSlots;
    int m_maxAttendees = 4;
};

#endif // COURT_H
