#include "class.h"

//setters
void Class::setCapacity(int capacity) 
{ this->capacity = capacity; }

void Class::setNumOfEnrolled(int numOfEnrolled) 
{ this->numOfEnrolled = numOfEnrolled; }

void Class::setTo(const QTime& to)
{this->to = to;}

void Class::setFrom(const QTime& from)
{this->from = from;}

void Class::setCoachName(const QString& coachName) 
{this->coachName = coachName;}

void Class::setClassName(const QString& className)
{this->className = className;}

void Class::setId(int id)
{this->id = id;}

//getters

QString Class::getClassName() const { return className; }
QString Class::getCoachName() const { return coachName; }
QTime Class::getFrom() const { return from; }
QTime Class::getTo() const { return to; }
int Class::getCapacity() const { return capacity; }
int Class::getNumOfEnrolled() const { return numOfEnrolled; }
int Class::getId() const {return id;}
}