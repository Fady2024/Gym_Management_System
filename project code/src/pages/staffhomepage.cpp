#include "staffhomepage.h"

StaffHomePage::StaffHomePage(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void StaffHomePage::setupUI()
{
    // Create a vertical layout
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    // Add a label
    auto label = new QLabel(tr("Add New Member!"), this);
    label->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; }");
    layout->addWidget(label);

    // Add a back button
    auto backButton = new QPushButton(tr("Back"), this);
    backButton->setStyleSheet("QPushButton { padding: 10px; font-size: 16px; }");
    layout->addWidget(backButton);

    // Connect button to signal
    connect(backButton, &QPushButton::clicked, this, &StaffHomePage::navigateBack);

    // Allow the layout to stretch
    layout->addStretch();
}