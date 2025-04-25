#include "timeLogic.h"

using namespace std;

// Constructor: starts the background thread to increment time  
TimeLogic::TimeLogic() {  
   currentTime = QDateTime::currentDateTime();
   keepRunning.store(true);
   multiplier = 1.0;  
   timeThread = thread(&TimeLogic::incrementTime, this); // Creates a thread running incrementTime in this object  
}

TimeLogic::~TimeLogic() {
    keepRunning = false; // Signal the thread to stop
    if (timeThread.joinable()) {
        timeThread.join(); // Ensure clean shutdown
    }
}
void TimeLogic::incrementTime() {
    while (keepRunning) {
        this_thread::sleep_for(chrono::milliseconds(int(1000 / multiplier))); // Sleep based on speed multiplier
		currentTime = currentTime.addSecs(1);
        cout << currentTime.toString().toStdString() << endl; // Print formatted time
    }
}
float TimeLogic::getMultiplier() {
    return multiplier;
}
// Function to set the speed multiplier
void TimeLogic::setMultiplier(float newMultiplier) {
    multiplier = newMultiplier;
}
/*
* Date components
    int year       = getCurrentTime().date().year();        // e.g., 2025
    int month      = getCurrentTime().date().month();       // [1, 12]
    int day        = getCurrentTime().date().day();         // [1, 31]
    int dayOfWeek  = getCurrentTime().date().dayOfWeek();   // [1, 7] (Monday = 1)
    int dayOfYear  = getCurrentTime().date().dayOfYear();   // [1, 365 or 366]

* Time components
    int hour       = getCurrentTime().time().hour();        // [0, 23]
    int minute     = getCurrentTime().time().minute();      // [0, 59]
    int second     = getCurrentTime().time().second();      // [0, 59]
    int msec       = getCurrentTime().time().msec();        // [0, 999]
*/
QDateTime TimeLogic::getCurrentTime() {
    return currentTime;
}
// Format: "Wed Oct 04 14:48:00 2023"
QString TimeLogic::getFormattedTime() {
	return currentTime.toString("ddd MMM dd hh:mm:ss yyyy");
}
TimeLogic timeLogicInstance; // Global instance of TimeLogic