# 🏋️‍♂️🎾 Gym & Padel Management System

## 🌟 Our Team

<div align="center">

### 💫 Team Members

<!-- Special Design for Fady Gerges (highlighted differently) -->
<p align="center">
  <strong>
    <a href="https://github.com/fady2024">
      <img src="https://img.shields.io/badge/Fady_Gerges_Kodsy_Al_Sagheer-%F0%9F%92%A5-blueviolet?style=for-the-badge&logo=github" alt="Fady Gerges" />
    </a>
  </strong>
</p>

<!-- Other Members -->
<p align="center">

  <a href="https://github.com/FadyEhab6">
    <img src="https://img.shields.io/badge/Fady_Ehab_Elsaid_Mohammed-303030?style=for-the-badge&logo=github&logoColor=white" alt="Fady Ehab" />
  </a>

  <a href="https://github.com/kareem-kio">
    <img src="https://img.shields.io/badge/Kareem_Amr_Mahmoud_Sultan-303030?style=for-the-badge&logo=github&logoColor=white" alt="Kareem Amr" />
  </a>

  <a href="https://github.com/Peter-Emad100">
    <img src="https://img.shields.io/badge/Peter_Emad_Adly_Shafik-303030?style=for-the-badge&logo=github&logoColor=white" alt="Peter Emad" />
  </a>

  <a href="https://github.com/SheikhWalter">
    <img src="https://img.shields.io/badge/Abdulrahman_Ali_Ahmed_Mohammed_Elkhayat-303030?style=for-the-badge&logo=github&logoColor=white" alt="Abdulrahman Ali" />
  </a>

  <a href="https://github.com/fatma0608">
    <img src="https://img.shields.io/badge/Fatma_Alzhraa_Ahmed_Sayed_Mohmed-303030?style=for-the-badge&logo=github&logoColor=white" alt="Fatma Alzhraa" />
  </a>

  <a href="https://github.com/Nouran252">
    <img src="https://img.shields.io/badge/Nouran_Mahmoud_Mohamed_Ismail-303030?style=for-the-badge&logo=github&logoColor=white" alt="Nouran Mahmoud" />
  </a>

</p>

</div>

## 📑 Table of Contents
1. [Features](#-features)
    - [User Management](#-user-management)
    - [Class Scheduling](#-class-scheduling)
    - [Workout Tracking](#-workout-tracking)
    - [Subscription Management](#-subscription-management)
    - [Padel Court Booking](#-padel-court-booking)
    - [Reporting System](#-reporting-system)
    - [VIP Features](#-vip-features)
2. [Technical Implementation](#-technical-implementation)
3. [Project Structure](#-project-structure)
4. [Installation](#-installation)

## ✨ Features

### 👥 User Management
- **Member Profiles**
    - ✅ Unique ID generation and management
    - ✅ Personal information storage (Name, DOB, Contact)
    - ✅ Profile picture support
    - ✅ Subscription status tracking
    - ✅ Workout history access

- **Staff Management**
    - 🔐 Role-based access control (Receptionist, Coach, Manager)
    - 📅 Staff scheduling and availability
    - 👤 Member information management
    - 🏋️‍♂️ Class assignment tracking
    - 📊 Performance monitoring

### 🕒 Class Scheduling
- **Class Management**
    - 📅 Monthly class scheduling
    - 🧮 Capacity tracking and management
    - 👨‍🏫 Coach assignment system
    - 🔄 Real-time availability updates
    - � Class type categorization

- **Waitlist System**
    - ⏳ Automatic waitlist management
    - 🔔 Priority-based notification system
    - 🆕 Real-time slot availability updates
    - ⭐ VIP member priority handling
    - ❌ Cancellation and rescheduling support

### 💪 Workout Tracking
- **Progress Monitoring**
    - 📈 Detailed workout history
    - 🎯 Performance metrics tracking
    - 🏆 Achievement system
    - 📊 Progress visualization
    - ✏️ Custom workout plans

### 💳 Subscription Management
- **Subscription Types**
    - 📅 Monthly plans
    - 🗓️ 3-month packages
    - 📆 6-month packages
    - 🎉 Yearly memberships
    - ⭐ VIP subscriptions

- **Renewal System**
    - 🔔 Automatic renewal reminders
    - 💰 Early renewal discounts
    - 💳 Payment processing
    - 📊 Subscription status tracking
    - ❌ Cancellation management

### 🎾 Padel Court Booking
- **Booking System**
    - ⏱️ Real-time court availability
    - 📅 Advanced booking options
    - 📍 Location-based search
    - ⏰ Time slot management
    - ❌ Cancellation policies

- **VIP Features**
    - ⭐ Priority booking access
    - 🕒 Exclusive time slots
    - 🏟️ Premium court selection
    - 📅 Extended booking windows
    - ❌ Special cancellation privileges

### 📊 Reporting System
- **Analytics Dashboard**
    - 👥 Member activity reports
    - 💰 Revenue tracking
    - 🏋️‍♂️ Class attendance statistics
    - 🎾 Court utilization metrics
    - 👨‍💼 Staff performance analysis

## 💻 Technical Implementation

### 🏗️ Data Structures
- **User Management**
    - 🗃️ Hash Maps for quick member lookup
    - ⏭️ Priority Queues for waitlist management
    - 🔗 Linked Lists for workout history

- **Scheduling System**
    - 🌳 Binary Search Trees for time slot management
    - 🚶‍♂️ Queues for class waitlists
    - 📊 Graphs for staff scheduling

- **Data Storage**
    - 📄 JSON files for persistent storage
    - 🔄 Efficient data serialization
    - ⚡ Optimized file I/O operations

### 🔒 Security Features
- 🔐 Encrypted user data storage
- 💳 Secure payment processing
- 👥 Role-based access control
- ⏱️ Session management
- 💾 Data backup system

## 🏗️ Project Structure

### 🧱 Core Components
- **DataManager/**
    - `userdatamanager.cpp/h`: Comprehensive user data operations
    - Implements secure CRUD operations
    - Data validation and sanitization

### 🔐 Authentication System
- **src/auth/**
    - `authpage.cpp/h`: Secure authentication system
    - Role-based access control
    - Session management

### 🖥️ UI Components
- **UI/**
    - `leftsidebar.cpp/h`: Navigation system
    - `TopPanel.cpp/h`: User interface controls
    - `UIUtils.cpp/h`: Common UI components

### 🌍 Language & Theme
- **Language/**
    - 🌐 Multi-language support
    - 🔄 Dynamic language switching

- **Theme/**
    - ☀️🌙 Light/Dark mode
    - 🎨 Customizable interface
    - ♿ Accessibility features

## ⚙️ Installation

1. **Prerequisites**
    - Qt 6.10 or later
    - C++17 compatible compiler
    - CMake 3.15 or later
    - Git

2. **Build Steps**
   ```bash
   git clone https://github.com/Fady2024/DS_Project.git
   cd gym-padel-system
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. **Initial Setup**
    - Configure database settings
    - Set up user permissions
    - Initialize system parameters
    - Create admin account

## License
This project is licensed under the GNU Affero General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
