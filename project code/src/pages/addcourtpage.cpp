#include "addcourtpage.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QSizePolicy>
#include <QTime>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

#include "Padel/Court.h"

AddCourtPage::AddCourtPage(PadelDataManager* padelManager, QWidget* parent)
    : QWidget(parent)
    , padelDataManager(padelManager)
    , isDarkTheme(false)
{
    setupUI();
    setupConnections();
    retranslateUI();
}

AddCourtPage::~AddCourtPage()
{
}

void AddCourtPage::setupUI()
{
    // Main layout with modern styling
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(24);

    QLabel* titleLabel = new QLabel(tr("Add New Court"));
    titleLabel->setObjectName("pageTitle");
    titleLabel->setStyleSheet("font-size: 28px; font-weight: 700; color: #F9FAFB; margin-bottom: 4px; background-color: transparent;");
    mainLayout->addWidget(titleLabel);
    
    QLabel* subtitleLabel = new QLabel(tr("Complete the form below to add a new padel court"));
    subtitleLabel->setStyleSheet("font-size: 16px; color: #9CA3AF; margin-bottom: 8px; background-color: transparent;");
    mainLayout->addWidget(subtitleLabel);

    // Content widget inside scroll area
    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    contentWidget = new QWidget();
    contentWidget->setObjectName("contentWidget");
    
    // Remove background color from contentWidget to prevent layering
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 12, 0);
    contentLayout->setSpacing(28);

    // Basic Info Card
    QWidget* basicInfoCard = new QWidget();
    basicInfoCard->setObjectName("basicInfoCard");
    basicInfoCard->setStyleSheet(
        "QWidget#basicInfoCard { "
        "   background-color: #1F2937; "
        "   border-radius: 16px; "
        "}"
    );
    
    QVBoxLayout* basicInfoLayout = new QVBoxLayout(basicInfoCard);
    basicInfoLayout->setContentsMargins(24, 24, 24, 32);
    basicInfoLayout->setSpacing(28);
    
    // Card title
    QLabel* basicInfoTitle = new QLabel(tr("Basic Information"));
    basicInfoTitle->setStyleSheet("font-size: 18px; font-weight: 600; color: #F9FAFB; margin-bottom: 6px;");
    basicInfoLayout->addWidget(basicInfoTitle);
    
    // Create a grid layout for the form fields with reduced spacing
    QGridLayout* formLayout = new QGridLayout();
    formLayout->setVerticalSpacing(20);
    formLayout->setHorizontalSpacing(28);
    formLayout->setColumnStretch(0, 1);
    formLayout->setColumnStretch(1, 1);
    
    // Court Name
    QLabel* nameLabel = new QLabel(tr("Court Name"));
    nameLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #F9FAFB;");
    QLabel* nameRequiredLabel = new QLabel(tr("*"));
    nameRequiredLabel->setStyleSheet("color: #EF4444; font-weight: bold;");
    
    QHBoxLayout* nameLabelLayout = new QHBoxLayout();
    nameLabelLayout->setContentsMargins(0, 0, 0, 0);
    nameLabelLayout->setSpacing(2);
    nameLabelLayout->addWidget(nameLabel);
    nameLabelLayout->addWidget(nameRequiredLabel);
    nameLabelLayout->addStretch();
    
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText(tr("Enter court name"));
    nameEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #374151;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QLineEdit:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #9CA3AF;"
        "}"
    );
    nameEdit->setMinimumHeight(34);
    
    nameErrorLabel = new QLabel(tr("Court name is required"));
    nameErrorLabel->setStyleSheet("color: #EF4444; font-size: 13px; margin-top: 4px;");
    nameErrorLabel->setVisible(false);
    
    QVBoxLayout* nameInputLayout = new QVBoxLayout();
    nameInputLayout->setContentsMargins(0, 0, 0, 0);
    nameInputLayout->setSpacing(6);
    nameInputLayout->addLayout(nameLabelLayout);
    nameInputLayout->addWidget(nameEdit);
    nameInputLayout->addWidget(nameErrorLabel);
    
    QLabel* locationLabel = new QLabel(tr("Location"));
    locationLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #F9FAFB;");
    QLabel* locationRequiredLabel = new QLabel(tr("*"));
    locationRequiredLabel->setStyleSheet("color: #EF4444; font-weight: bold;");
    
    QHBoxLayout* locationLabelLayout = new QHBoxLayout();
    locationLabelLayout->setContentsMargins(0, 0, 0, 0);
    locationLabelLayout->setSpacing(2);
    locationLabelLayout->addWidget(locationLabel);
    locationLabelLayout->addWidget(locationRequiredLabel);
    locationLabelLayout->addStretch();
    
    locationComboBox = new QComboBox();
    locationComboBox->setEditable(true);
    locationComboBox->setPlaceholderText(tr("Select location"));
    locationComboBox->addItems(QStringList() << "Cairo" << "Madrid" << "Valencia" << "Seville" << "Malaga");
    locationComboBox->setCurrentIndex(0);
    locationComboBox->setStyleSheet(
        "QComboBox {"
        "   border: 1px solid #374151;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 13px;"
        "   min-height: 34px;"
        "}"
        "QComboBox:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QComboBox:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QComboBox::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: center right;"
        "   width: 22px;"
        "   border-left-width: 0px;"
        "   border-top-right-radius: 10px;"
        "   border-bottom-right-radius: 10px;"
        "}"
        "QComboBox::down-arrow {"
        "   image: url(:/Images/dropdown_arrow.png);"
        "   width: 12px;"
        "   height: 12px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   border: 1px solid #374151;"
        "   border-radius: 8px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   selection-background-color: #818CF8;"
        "   selection-color: white;"
        "}"
    );
    
    locationErrorLabel = new QLabel(tr("Location is required"));
    locationErrorLabel->setStyleSheet("color: #EF4444; font-size: 13px; margin-top: 4px;");
    locationErrorLabel->setVisible(false);
    
    QVBoxLayout* locationInputLayout = new QVBoxLayout();
    locationInputLayout->setContentsMargins(0, 0, 0, 0);
    locationInputLayout->setSpacing(6);
    locationInputLayout->addLayout(locationLabelLayout);
    locationInputLayout->addWidget(locationComboBox);
    locationInputLayout->addWidget(locationErrorLabel);
    
    // Price per hour
    QLabel* priceLabel = new QLabel(tr("Price Per Hour (€)"));
    priceLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #F9FAFB;");
    QLabel* priceRequiredLabel = new QLabel(tr("*"));
    priceRequiredLabel->setStyleSheet("color: #EF4444; font-weight: bold;");
    
    QHBoxLayout* priceLabelLayout = new QHBoxLayout();
    priceLabelLayout->setContentsMargins(0, 0, 0, 0);
    priceLabelLayout->setSpacing(2);
    priceLabelLayout->addWidget(priceLabel);
    priceLabelLayout->addWidget(priceRequiredLabel);
    priceLabelLayout->addStretch();
    
    priceSpinBox = new QDoubleSpinBox();
    priceSpinBox->setMinimum(0.00);
    priceSpinBox->setMaximum(999.99);
    priceSpinBox->setValue(0.00);
    priceSpinBox->setDecimals(2);
    priceSpinBox->setSingleStep(5.00);
    priceSpinBox->setPrefix("€ ");
    priceSpinBox->setStyleSheet(
        "QDoubleSpinBox {"
        "   border: 1px solid #374151;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 13px;"
        "   min-height: 34px;"
        "}"
        "QDoubleSpinBox:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QDoubleSpinBox:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        "   width: 16px;"
        "   height: 16px;"
        "   border-radius: 3px;"
        "   background-color: #2D3748;"
        "   margin-right: 5px;"
        "}"
        "QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {"
        "   background-color: #4B5563;"
        "}"
        "QDoubleSpinBox::up-arrow {"
        "   image: url(:/Images/arrow_up.png);"
        "   width: 10px;"
        "   height: 10px;"
        "}"
        "QDoubleSpinBox::down-arrow {"
        "   image: url(:/Images/arrow_down.png);"
        "   width: 10px;"
        "   height: 10px;"
        "}"
    );
    
    priceErrorLabel = new QLabel(tr("Price must be greater than 0"));
    priceErrorLabel->setStyleSheet("color: #EF4444; font-size: 13px; margin-top: 4px;");
    priceErrorLabel->setVisible(false);
    
    QVBoxLayout* priceInputLayout = new QVBoxLayout();
    priceInputLayout->setContentsMargins(0, 0, 0, 0);
    priceInputLayout->setSpacing(6);
    priceInputLayout->addLayout(priceLabelLayout);
    priceInputLayout->addWidget(priceSpinBox);
    priceInputLayout->addWidget(priceErrorLabel);
    
    // Max Attendees
    QLabel* attendeesLabel = new QLabel(tr("Maximum Attendees"));
    attendeesLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #F9FAFB;");
    
    QHBoxLayout* attendeesLabelLayout = new QHBoxLayout();
    attendeesLabelLayout->setContentsMargins(0, 0, 0, 0);
    attendeesLabelLayout->setSpacing(2);
    attendeesLabelLayout->addWidget(attendeesLabel);
    attendeesLabelLayout->addStretch();
    
    attendeesSpinBox = new QSpinBox();
    attendeesSpinBox->setMinimum(1);
    attendeesSpinBox->setMaximum(20);
    attendeesSpinBox->setValue(4); // Default to 4 attendees
    attendeesSpinBox->setStyleSheet(
        "QSpinBox {"
        "   border: 1px solid #374151;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 13px;"
        "   min-height: 34px;"
        "}"
        "QSpinBox:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QSpinBox:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "   width: 16px;"
        "   height: 16px;"
        "   border-radius: 3px;"
        "   background-color: #2D3748;"
        "   margin-right: 5px;"
        "}"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
        "   background-color: #4B5563;"
        "}"
        "QSpinBox::up-arrow {"
        "   image: url(:/Images/chevron-up.png);"
        "   width: 14px;"
        "   height: 14px;"
        "}"
        "QSpinBox::down-arrow {"
        "   image: url(:/Images/chevron-down.png);"
        "   width: 14px;"
        "   height: 14px;"
        "}"
    );
    
    QLabel* attendeesInfoLabel = new QLabel(tr("Max players allowed on this court"));
    attendeesInfoLabel->setStyleSheet("color: #9CA3AF; font-size: 13px; margin-top: 4px;");
    
    QVBoxLayout* attendeesInputLayout = new QVBoxLayout();
    attendeesInputLayout->setContentsMargins(0, 0, 0, 0);
    attendeesInputLayout->setSpacing(6);
    attendeesInputLayout->addLayout(attendeesLabelLayout);
    attendeesInputLayout->addWidget(attendeesSpinBox);
    attendeesInputLayout->addWidget(attendeesInfoLabel);
    
    // Court Type
    QLabel* typeLabel = new QLabel(tr("Court Type"));
    typeLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #F9FAFB;");
    
    QHBoxLayout* typeLabelLayout = new QHBoxLayout();
    typeLabelLayout->setContentsMargins(0, 0, 0, 0);
    typeLabelLayout->addWidget(typeLabel);
    typeLabelLayout->addStretch();
    
    courtTypeGroup = new QButtonGroup(this);
    courtTypeGroup->setExclusive(true);
    
    QWidget* typeSlider = new QWidget();
    typeSlider->setObjectName("typeSlider");
    typeSlider->setFixedHeight(40);
    
    QHBoxLayout* sliderLayout = new QHBoxLayout(typeSlider);
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->setSpacing(0);
    
    // Create the two radio buttons
    indoorRadio = new QRadioButton(tr("Indoor"));
    indoorRadio->setObjectName("indoorRadio");
    indoorRadio->setChecked(true);
    indoorRadio->setCursor(Qt::PointingHandCursor);
    indoorRadio->setMinimumWidth(100);
    
    outdoorRadio = new QRadioButton(tr("Outdoor"));
    outdoorRadio->setObjectName("outdoorRadio");
    outdoorRadio->setCursor(Qt::PointingHandCursor);
    outdoorRadio->setMinimumWidth(100);
    
    // Add buttons to the group
    courtTypeGroup->addButton(indoorRadio);
    courtTypeGroup->addButton(outdoorRadio);
    
    // Connect to a fast signal handler
    connect(indoorRadio, &QRadioButton::toggled, this, &AddCourtPage::onCourtTypeToggled);
    connect(outdoorRadio, &QRadioButton::toggled, this, &AddCourtPage::onCourtTypeToggled);
    
    // Add buttons to the layout
    sliderLayout->addWidget(indoorRadio);
    sliderLayout->addWidget(outdoorRadio);
    
    QVBoxLayout* typeInputLayout = new QVBoxLayout();
    typeInputLayout->setContentsMargins(0, 0, 0, 6);
    typeInputLayout->setSpacing(4);
    typeInputLayout->addLayout(typeLabelLayout);
    typeInputLayout->addWidget(typeSlider);
    
    // Add inputs to the form layout
    formLayout->addLayout(nameInputLayout, 0, 0);
    formLayout->addLayout(locationInputLayout, 0, 1);
    formLayout->addLayout(priceInputLayout, 1, 0);
    formLayout->addLayout(attendeesInputLayout, 1, 1);
    formLayout->addLayout(typeInputLayout, 2, 0, 1, 2);
    
    basicInfoLayout->addLayout(formLayout);
    contentLayout->addWidget(basicInfoCard);

    // Setup other sections
    setupFeatureCards();
    setupTimeSlotButtons();

    // Description Card with improved styling
    QWidget* descriptionCard = new QWidget();
    descriptionCard->setObjectName("descriptionCard");
    descriptionCard->setStyleSheet(
        "QWidget#descriptionCard { "
        "   background-color: #1F2937; "
        "   border-radius: 16px; "
        "}"
    );
    
    QVBoxLayout* descriptionLayout = new QVBoxLayout(descriptionCard);
    descriptionLayout->setContentsMargins(28, 28, 28, 28);
    descriptionLayout->setSpacing(20);
    
    QLabel* descriptionTitle = new QLabel(tr("Description"));
    descriptionTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #F9FAFB;");
    descriptionLayout->addWidget(descriptionTitle);
    
    descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText(tr("Enter a detailed description of the court..."));
    descriptionEdit->setStyleSheet(
        "QTextEdit {"
        "   border: 1px solid #374151;"
        "   border-radius: 10px;"
        "   padding: 12px 16px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 15px;"
        "}"
        "QTextEdit:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QTextEdit:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QTextEdit::placeholder {"
        "   color: #9CA3AF;"
        "}"
    );
    descriptionEdit->setMinimumHeight(180);
    descriptionLayout->addWidget(descriptionEdit);
    
    // Character count with cleaner layout
    QHBoxLayout* charCountLayout = new QHBoxLayout();
    charCountLayout->setContentsMargins(0, 0, 0, 0);
    charCountLayout->setAlignment(Qt::AlignRight);
    
    QLabel* descInfoLabel = new QLabel(tr("Provide details about the court surface, rules, or any special instructions."));
    descInfoLabel->setStyleSheet("color: #9CA3AF; font-size: 13px;");
    
    charCountLabel = new QLabel(tr("0/500"));
    charCountLabel->setStyleSheet("color: #9CA3AF; font-size: 13px; font-weight: 500;");
    
    charCountLayout->addWidget(descInfoLabel);
    charCountLayout->addStretch();
    charCountLayout->addWidget(charCountLabel);
    
    descriptionLayout->addLayout(charCountLayout);
    contentLayout->addWidget(descriptionCard);
    
    // Action Buttons
    QHBoxLayout* actionButtonsLayout = new QHBoxLayout();
    actionButtonsLayout->setContentsMargins(0, 16, 0, 0);
    actionButtonsLayout->setSpacing(16);
    
    cancelButton = new QPushButton(tr("Reset Form"));
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #374151;"
        "   color: #D1D5DB;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 14px 28px;"
        "   font-size: 15px;"
        "   font-weight: 500;"
        "   min-height: 50px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4B5563;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1F2937;"
        "}"
    );
    
    addCourtButton = new QPushButton(tr("Add Court"));
    addCourtButton->setCursor(Qt::PointingHandCursor);
    addCourtButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #818CF8, stop:1 #6366F1);"
        "   color: #FFFFFF;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 14px 28px;"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   min-height: 50px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366F1, stop:1 #4F46E5);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4F46E5, stop:1 #4F46E5);"
        "}"
    );
    
    actionButtonsLayout->addWidget(cancelButton);
    actionButtonsLayout->addStretch();
    actionButtonsLayout->addWidget(addCourtButton);
    
    contentLayout->addLayout(actionButtonsLayout);
    
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // Set default dark theme
    setStyleSheet("background-color: #111827;");
    
    // Set scroll area style
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollBar:vertical {"
        "   background-color: transparent;"
        "   width: 8px;"
        "   margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #4B5563;"
        "   min-height: 30px;"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #6B7280;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "   background: none;"
        "}"
    );

    // Set initial styles
    updateTheme(isDarkTheme);
}

void AddCourtPage::setupConnections()
{
    connect(addCourtButton, &QPushButton::clicked, this, &AddCourtPage::onAddCourtClicked);
    connect(cancelButton, &QPushButton::clicked, this, &AddCourtPage::onCancelClicked);
    connect(addFeatureButton, &QPushButton::clicked, this, &AddCourtPage::onAddFeatureClicked);
    
    // Connect validation signals
    connect(nameEdit, &QLineEdit::textChanged, [this](const QString& text) {
        validateField(nameEdit, !text.trimmed().isEmpty(), tr("Court name is required"));
    });
    
    connect(locationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        locationErrorLabel->setVisible(index == -1 && locationComboBox->currentText().trimmed().isEmpty());
    });
    
    connect(locationComboBox, &QComboBox::editTextChanged, [this](const QString& text) {
        locationErrorLabel->setVisible(text.trimmed().isEmpty());
    });
    
    connect(priceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        priceErrorLabel->setVisible(value <= 0);
    });
    
    connect(descriptionEdit, &QTextEdit::textChanged, this, &AddCourtPage::updateCharCount);
}

void AddCourtPage::onAddCourtClicked()
{
    // Validate inputs
    bool isValid = true;
    
    // Validate court name
    if (nameEdit->text().trimmed().isEmpty()) {
        validateField(nameEdit, false, tr("Court name is required"));
        isValid = false;
    }

    // Validate location
    if (locationComboBox->currentText().trimmed().isEmpty()) {
        locationErrorLabel->setVisible(true);
        isValid = false;
    }
    
    // Validate price
    if (priceSpinBox->value() <= 0) {
        priceErrorLabel->setVisible(true);
        isValid = false;
    }

    // Validate time slots
    if (selectedTimeSlots.isEmpty()) {
        timeSlotErrorLabel->setVisible(true);
        isValid = false;
    }
    
    if (!isValid) {
        return;
    }

    // Create court object
    Court court;
    court.setName(nameEdit->text().trimmed());
    court.setLocation(locationComboBox->currentText().trimmed());
    court.setIndoor(indoorRadio->isChecked());
    court.setPricePerHour(priceSpinBox->value());
    court.setDescription(descriptionEdit->toPlainText().trimmed());
    court.setMaxAttendees(attendeesSpinBox->value());

    // Set features
    court.setFeatures(selectedFeatures);

    // Set time slots
    std::vector<QTime>& timeSlots = court.getAllTimeSlots();
    for (const QString& timeSlotStr : selectedTimeSlots) {
        QTime time = QTime::fromString(timeSlotStr, "HH:mm");
        if (time.isValid()) {
            timeSlots.push_back(time);
        }
    }

    // Add court to database
    QString errorMessage;
    if (padelDataManager->addCourt(court, errorMessage)) {
        showSuccessMessage(tr("Court added successfully!"));
        clearForm();
    } else {
        showErrorMessage(tr("Failed to add court: ") + errorMessage);
    }
}

void AddCourtPage::onCancelClicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Reset Form"));
    msgBox.setText(tr("Are you sure you want to reset all fields?"));
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() == QMessageBox::Yes) {
        clearForm();
    }
}

void AddCourtPage::onAddFeatureClicked()
{
    QString feature = featureEdit->text().trimmed();
    if (!feature.isEmpty()) {
        // Check if this feature already exists
        if (featureCards.contains(feature)) {
            featureCards[feature]->setSelected(true);
            if (!selectedFeatures.contains(feature)) {
                selectedFeatures.append(feature);
            }
        } else {
            // Create a new feature card
            FeatureCard* card = new FeatureCard("✨", feature);
            featureCards[feature] = card;
            connect(card, &FeatureCard::toggled, this, &AddCourtPage::onFeatureToggled);
            card->setSelected(true);
            selectedFeatures.append(feature);
            
            // Add to the grid layout
            QGridLayout* gridLayout = qobject_cast<QGridLayout*>(featuresContainer->layout());
            int itemCount = gridLayout->count();
            int row = itemCount / 3;
            int col = itemCount % 3;
            gridLayout->addWidget(card, row, col);
        }
        
        featureEdit->clear();
    }
}

void AddCourtPage::clearForm()
{
    // Reset basic fields
    nameEdit->clear();
    locationComboBox->setCurrentIndex(-1);
    indoorRadio->setChecked(true);
    priceSpinBox->setValue(0.0);
    attendeesSpinBox->setValue(4); // Reset to default
    
    // Reset features
    selectedFeatures.clear();
    for (FeatureCard* card : featureCards) {
        card->setSelected(false);
    }
    
    // Reset time slots
    selectedTimeSlots.clear();
    for (TimeSlotButton* button : timeSlotButtons) {
        button->setChecked(false);
    }
    
    // Reset description
    descriptionEdit->clear();
    updateCharCount();
    
    // Hide error labels
    nameErrorLabel->setVisible(false);
    locationErrorLabel->setVisible(false);
    priceErrorLabel->setVisible(false);
    timeSlotErrorLabel->setVisible(false);
}

void AddCourtPage::showSuccessMessage(const QString& message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Success"));
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.exec();
}

void AddCourtPage::showErrorMessage(const QString& message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Error"));
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
}

void AddCourtPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;
    
    // Set the base color scheme based on the theme
    QString bgColor = isDark ? "#111827" : "#F9FAFB";
    QString cardBgColor = isDark ? "#1F2937" : "#FFFFFF";
    QString textColor = isDark ? "#F9FAFB" : "#111827";
    QString secondaryTextColor = isDark ? "#9CA3AF" : "#6B7280";
    QString borderColor = isDark ? "#374151" : "#E5E7EB";
    QString inputBgColor = isDark ? "#1E293B" : "#FFFFFF";
    QString primaryColor = isDark ? "#818CF8" : "#6366F1";
    QString errorColor = "#EF4444";
    QString buttonBgColor = isDark ? "#374151" : "#F3F4F6";
    QString buttonTextColor = isDark ? "#D1D5DB" : "#4B5563";
    QString buttonHoverColor = isDark ? "#4B5563" : "#E5E7EB";
    
    // Light mode specific colors
    QString lightModeSliderBg = "#F3F4F6";
    QString lightModeUnselectedBg = "#E5E7EB";
    QString lightModeUnselectedText = "#4B5563";
    QString lightModeSelectedText = "#111827";
    
    // Dark mode specific colors
    QString darkModeSliderBg = "#1E293B";
    QString darkModeUnselectedBg = "#2D3748";
    QString darkModeUnselectedText = "#D1D5DB";
    QString darkModeSelectedText = "#FFFFFF";
    
    // Apply themed styles to widgets
    setStyleSheet("");
    
    // Simplify cards design
    QList<QWidget*> cards = {
        findChild<QWidget*>("basicInfoCard"),
        findChild<QWidget*>("featuresCard"),
        findChild<QWidget*>("timeSlotsCard"),
        findChild<QWidget*>("descriptionCard")
    };
    
    for (QWidget* card : cards) {
        if (card) {
            card->setStyleSheet(QString(
                "QWidget { "
                "   background-color: %1; "
                "   border-radius: 16px; "
                "}"
            ).arg(cardBgColor));
        }
    }
    
    // Update content widget background
    if (contentWidget) {
        contentWidget->setStyleSheet("QWidget#contentWidget { background-color: transparent; }");
    }
    
    // Update labels
    QList<QLabel*> allLabels = findChildren<QLabel*>();
    for (QLabel* label : allLabels) {
        if (label->objectName() == "pageTitle") {
            label->setStyleSheet(QString("font-size: 28px; font-weight: 700; color: %1; margin-bottom: 4px; background-color: transparent;").arg(textColor));
        }
        else if (label->text().contains("*")) {
            // Required labels
            label->setStyleSheet(QString("color: %1; font-weight: bold; background-color: transparent;").arg(errorColor));
        }
        else if (label->text() == "Basic Information" || label->text() == "Court Features" || 
                 label->text() == "Available Time Slots" || label->text() == "Description") {
            // Section titles
            label->setStyleSheet(QString("font-size: 20px; font-weight: 600; color: %1; background-color: transparent;").arg(textColor));
        }
        else if (label->text().contains("required") || label->text().contains("greater") || 
                 label->text().contains("Select at least")) {
            // Error messages
            label->setStyleSheet(QString("color: %1; font-size: 13px; margin-top: 4px; background-color: transparent;").arg(errorColor));
        }
        else if (label->text().contains("details") || label->text() == charCountLabel->text()) {
            // Info text
            label->setStyleSheet(QString("color: %1; font-size: 13px; background-color: transparent;").arg(secondaryTextColor));
        }
        else if (label->text() == "Court Name" || label->text() == "Location" || 
                 label->text() == "Price Per Hour (€)" || label->text() == "Court Type") {
            // Form labels
            label->setStyleSheet(QString("font-size: 13px; font-weight: 500; color: %1; background-color: transparent;").arg(textColor));
        }
        else {
            // Default label style
            label->setStyleSheet(QString("color: %1; font-size: 14px; background-color: transparent;").arg(textColor));
        }
    }
    
    // Update input fields
    QList<QLineEdit*> lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* edit : lineEdits) {
        edit->setStyleSheet(QString(
            "QLineEdit {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;"
            "}"
            "QLineEdit:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QLineEdit:hover {"
            "   border: 1px solid %5;"
            "}"
            "QLineEdit::placeholder {"
            "   color: %6;"
            "}"
        ).arg(borderColor).arg(inputBgColor).arg(textColor)
          .arg(primaryColor).arg(isDark ? "#4B5563" : "#D1D5DB").arg(secondaryTextColor));
    }
    
    // Update combo boxes
    QList<QComboBox*> comboBoxes = findChildren<QComboBox*>();
    for (QComboBox* combo : comboBoxes) {
        combo->setStyleSheet(QString(
            "QComboBox {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;"
            "}"
            "QComboBox:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QComboBox:hover {"
            "   border: 1px solid %5;"
            "}"
            "QComboBox::drop-down {"
            "   subcontrol-origin: padding;"
            "   subcontrol-position: center right;"
            "   width: 22px;"
            "   border-left-width: 0px;"
            "   border-top-right-radius: 10px;"
            "   border-bottom-right-radius: 10px;"
            "}"
            "QComboBox::down-arrow {"
            "   image: url(:/Images/dropdown_arrow.png);"
            "   width: 12px;"
            "   height: 12px;"
            "}"
            "QComboBox QAbstractItemView {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   background-color: %2;"
            "   color: %3;"
            "   selection-background-color: %4;"
            "   selection-color: white;"
            "}"
        ).arg(borderColor).arg(inputBgColor).arg(textColor)
          .arg(primaryColor).arg(isDark ? "#4B5563" : "#D1D5DB"));
    }
    
    // Update spin boxes
    QList<QDoubleSpinBox*> doubleSpinBoxes = findChildren<QDoubleSpinBox*>();
    for (QDoubleSpinBox* spinBox : doubleSpinBoxes) {
        spinBox->setStyleSheet(QString(
            "QDoubleSpinBox {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;"
            "}"
            "QDoubleSpinBox:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QDoubleSpinBox:hover {"
            "   border: 1px solid %5;"
            "}"
            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
            "   width: 16px;"
            "   height: 16px;"
            "   border-radius: 3px;"
            "   background-color: %6;"
            "   margin-right: 5px;"
            "}"
            "QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {"
            "   background-color: %7;"
            "}"
            "QDoubleSpinBox::up-arrow {"
            "   image: url(:/Images/chevron-up.png);"
            "   width: 14px;"
            "   height: 14px;"
            "}"
            "QDoubleSpinBox::down-arrow {"
            "   image: url(:/Images/chevron-down.png);"
            "   width: 14px;"
            "   height: 14px;"
            "}"
        ).arg(borderColor).arg(inputBgColor).arg(textColor)
          .arg(primaryColor).arg(isDark ? "#4B5563" : "#D1D5DB")
          .arg(isDark ? "#2D3748" : "#F3F4F6").arg(isDark ? "#4B5563" : "#E5E7EB"));
    }
    
    // Update integer spin boxes
    QList<QSpinBox*> spinBoxes = findChildren<QSpinBox*>();
    for (QSpinBox* spinBox : spinBoxes) {
        spinBox->setStyleSheet(QString(
            "QSpinBox {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;"
            "}"
            "QSpinBox:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QSpinBox:hover {"
            "   border: 1px solid %5;"
            "}"
            "QSpinBox::up-button, QSpinBox::down-button {"
            "   width: 16px;"
            "   height: 16px;"
            "   border-radius: 3px;"
            "   background-color: %6;"
            "   margin-right: 5px;"
            "}"
            "QSpinBox::up-button:hover, QSpinBox::down-button:hover {"
            "   background-color: %7;"
            "}"
            "QSpinBox::up-arrow {"
            "   image: url(:/Images/chevron-up.png);"
            "   width: 14px;"
            "   height: 14px;"
            "}"
            "QSpinBox::down-arrow {"
            "   image: url(:/Images/chevron-down.png);"
            "   width: 14px;"
            "   height: 14px;"
            "}"
        ).arg(borderColor).arg(inputBgColor).arg(textColor)
          .arg(primaryColor).arg(isDark ? "#4B5563" : "#D1D5DB")
          .arg(isDark ? "#2D3748" : "#F3F4F6").arg(isDark ? "#4B5563" : "#E5E7EB"));
    }
    
    // Update text edit with consistent styling
    if (descriptionEdit) {
        descriptionEdit->setStyleSheet(QString(
            "QTextEdit {"
            "   border: 1px solid %1;"
            "   border-radius: 10px;"
            "   padding: 12px 16px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 15px;"
            "}"
            "QTextEdit:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QTextEdit:hover {"
            "   border: 1px solid %5;"
            "}"
            "QTextEdit::placeholder {"
            "   color: %6;"
            "}"
        ).arg(borderColor).arg(inputBgColor).arg(textColor)
          .arg(primaryColor).arg(isDark ? "#4B5563" : "#D1D5DB").arg(secondaryTextColor));
    }
    
    // Create a special style for the type slider container
    QWidget* typeSlider = findChild<QWidget*>("typeSlider");
    if (typeSlider) {
        typeSlider->setStyleSheet(QString(
            "QWidget#typeSlider {"
            "   background-color: %1;"
            "   border-radius: 20px;"
            "   padding: 0px;"
            "}"
        ).arg(isDark ? darkModeSliderBg : lightModeSliderBg));
        
        typeSlider->setFixedHeight(40);
    }
    
    // Direct styling for indoor/outdoor buttons
    indoorRadio->setStyleSheet(QString(
        "QRadioButton {"
        "   font-size: 13px;"
        "   color: %1;"
        "   spacing: 0px;"
        "   padding: 0px 12px;"
        "   background-color: %2;"
        "   text-align: center;"
        "   margin: 0px;"
        "   min-height: 40px;"
        "   border-top-left-radius: 20px;"
        "   border-bottom-left-radius: 20px;"
        "   border-top-right-radius: 0px;"
        "   border-bottom-right-radius: 0px;"
        "   border: 1px solid %3;"
        "   qproperty-alignment: AlignCenter;"
        "}"
        "QRadioButton:checked {"
        "   background-color: %4;"
        "   color: %5;"
        "   font-weight: 700;"
        "   border: 2px solid %6;"
        "}"
        "QRadioButton::indicator {"
        "   width: 0px;"
        "   height: 0px;"
        "}"
    ).arg(isDark ? darkModeUnselectedText : lightModeUnselectedText)
      .arg(isDark ? darkModeUnselectedBg : lightModeUnselectedBg)
      .arg(isDark ? "#4A5568" : "#CBD5E0")
      .arg(isDark ? primaryColor : "#F1F5F9")
      .arg(isDark ? darkModeSelectedText : lightModeSelectedText)
      .arg(primaryColor));
    
    // Outdoor Radio Button
    outdoorRadio->setStyleSheet(QString(
        "QRadioButton {"
        "   font-size: 13px;"
        "   color: %1;"
        "   spacing: 0px;"
        "   padding: 0px 12px;"
        "   background-color: %2;"
        "   text-align: center;"
        "   margin: 0px;"
        "   min-height: 40px;"
        "   border-top-left-radius: 0px;"
        "   border-bottom-left-radius: 0px;"
        "   border-top-right-radius: 20px;"
        "   border-bottom-right-radius: 20px;"
        "   border: 1px solid %3;"
        "   qproperty-alignment: AlignCenter;"
        "}"
        "QRadioButton:checked {"
        "   background-color: %4;"
        "   color: %5;"
        "   font-weight: 700;"
        "   border: 2px solid %6;"
        "}"
        "QRadioButton::indicator {"
        "   width: 0px;"
        "   height: 0px;"
        "}"
    ).arg(isDark ? darkModeUnselectedText : lightModeUnselectedText)
      .arg(isDark ? darkModeUnselectedBg : lightModeUnselectedBg)
      .arg(isDark ? "#4A5568" : "#CBD5E0")
      .arg(isDark ? primaryColor : "#F1F5F9")
      .arg(isDark ? darkModeSelectedText : lightModeSelectedText)
      .arg(primaryColor));
    
    // Update time slot buttons with theme-specific styling
    for (TimeSlotButton* button : timeSlotButtons.values()) {
        button->setStyleSheet(QString(
            "QPushButton {"
            "   background-color: %1;"
            "   color: %2;"
            "   border: none;"
            "   border-radius: 20px;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: %3;"
            "}"
            "QPushButton:checked {"
            "   background-color: %4;"
            "   color: %5;"
            "}"
        ).arg(isDark ? darkModeUnselectedBg : lightModeUnselectedBg)
          .arg(isDark ? darkModeUnselectedText : lightModeUnselectedText)
          .arg(isDark ? "#374151" : "#D1D5DB")
          .arg(primaryColor)
          .arg(isDark ? darkModeSelectedText : "#FFFFFF"));
    }
    
    // Update feature cards styling to respect theme
    for (FeatureCard* card : featureCards.values()) {
        card->setIsDarkTheme(isDark);
        card->update();
    }
    
    this->setStyleSheet(QString("background-color: %1;").arg(bgColor));
}

void AddCourtPage::updateLayout()
{
    // Adjust UI based on window size
    int windowWidth = width();
    
    if (windowWidth < 768) {
        // For smaller screens
        mainLayout->setContentsMargins(16, 16, 16, 16);
        mainLayout->setSpacing(16);
    } else {
        // For larger screens
        mainLayout->setContentsMargins(24, 24, 24, 24);
        mainLayout->setSpacing(20);
    }
}

void AddCourtPage::retranslateUI()
{
    // Update all text elements for internationalization
    QList<QLabel*> allLabels = findChildren<QLabel*>();
    for (QLabel* label : allLabels) {
        if (label->objectName() == "pageTitle") {
            label->setText(tr("Add New Court"));
        }
    }
    
    // Update button text
    addCourtButton->setText(tr("Add Court"));
    cancelButton->setText(tr("Reset Form"));
    
    // Update placeholders
    nameEdit->setPlaceholderText(tr("Enter court name"));
    locationComboBox->setPlaceholderText(tr("Select location"));
    featureEdit->setPlaceholderText(tr("Add custom feature..."));
    descriptionEdit->setPlaceholderText(tr("Enter a detailed description of the court..."));
}

// FeatureCard Implementation
FeatureCard::FeatureCard(const QString& icon, const QString& text, QWidget* parent)
    : QWidget(parent)
    , selected(false)
    , isDarkTheme(true)
    , featureText(text)
{
    setFixedHeight(50);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(10);
    
    iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet("font-size: 18px;");
    
    textLabel = new QLabel(text);
    textLabel->setStyleSheet("font-size: 14px; font-weight: 500;");
    
    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addStretch();
    
    setStyleSheet("background-color: transparent; border-radius: 10px;");
}

void FeatureCard::setSelected(bool value) {
    if (selected != value) {
        selected = value;
        update(); // Trigger repaint
    }
}

void FeatureCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        selected = !selected;
        update();
        emit toggled(selected);
    }
    QWidget::mousePressEvent(event);
}

void FeatureCard::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    
    if (selected) {
        // Selected style - gradient background
        QLinearGradient gradient(0, 0, width(), 0);
        gradient.setColorAt(0, QColor("#4F46E5"));
        gradient.setColorAt(1, QColor("#6366F1"));
        
        painter.fillPath(path, gradient);
        painter.setPen(QPen(QColor("#818CF8"), 1));
        iconLabel->setStyleSheet("font-size: 20px;");
        textLabel->setStyleSheet("font-size: 14px; font-weight: 600; color: #FFFFFF;");
    } else {
        // Unselected style - respect theme
        if (isDarkTheme) {
            painter.fillPath(path, QColor("#2D3748")); // Dark background
            painter.setPen(QPen(QColor("#4B5563"), 0));
            iconLabel->setStyleSheet("font-size: 20px;");
            textLabel->setStyleSheet("font-size: 14px; font-weight: 500; color: #D1D5DB;");
        } else {
            painter.fillPath(path, QColor("#F3F4F6")); // Light background
            painter.setPen(QPen(QColor("#D1D5DB"), 0));
            iconLabel->setStyleSheet("font-size: 20px;");
            textLabel->setStyleSheet("font-size: 14px; font-weight: 500; color: #4B5563;");
        }
    }
    
    painter.drawPath(path);
    
    QWidget::paintEvent(event);
}

void FeatureCard::setIsDarkTheme(bool isDark) {
    isDarkTheme = isDark;
    update();
}

// TimeSlotButton Implementation
TimeSlotButton::TimeSlotButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setCheckable(true);
    setFixedHeight(40);
    setFixedWidth(80);
    setCursor(Qt::PointingHandCursor);
    
    setStyleSheet(
        "QPushButton {"
        "   background-color: #F9FAFB;"
        "   color: #4B5563;"
        "   border: 1px solid #E5E7EB;"
        "   border-radius: 20px;"
        "   font-size: 14px;"
        "   font-weight: 500;"
        "   padding: 6px 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F3F4F6;"
        "   border: 1px solid #D1D5DB;"
        "}"
        "QPushButton:checked {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6366F1, stop:1 #4F46E5);"
        "   color: #FFFFFF;"
        "   border: none;"
        "}"
    );
}

void AddCourtPage::setupFeatureCards()
{
    // Create Court Features card with modern styling
    QWidget* featuresCard = new QWidget();
    featuresCard->setObjectName("featuresCard");
    featuresCard->setStyleSheet(
        "QWidget#featuresCard { "
        "   background-color: #1F2937; " // Default to dark theme
        "   border-radius: 16px; "
        "}"
    );
    
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresCard);
    featuresLayout->setContentsMargins(28, 28, 28, 28);
    featuresLayout->setSpacing(20);
    
    QLabel* featuresTitle = new QLabel(tr("Court Features"));
    featuresTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #F9FAFB;");
    featuresLayout->addWidget(featuresTitle);
    
    // Features Grid
    featuresContainer = new QWidget();
    QGridLayout* featuresGridLayout = new QGridLayout(featuresContainer);
    featuresGridLayout->setContentsMargins(0, 0, 0, 0);
    featuresGridLayout->setVerticalSpacing(16);
    featuresGridLayout->setHorizontalSpacing(16);
    
    // Create feature cards with visually appealing icons
    featureCards.clear();
    featureCards["Lighting"] = new FeatureCard("💡", tr("Lighting"));
    featureCards["Glass Walls"] = new FeatureCard("🪟", tr("Glass Walls"));
    featureCards["Panoramic View"] = new FeatureCard("🏞️", tr("Panoramic View"));
    featureCards["Locker Room"] = new FeatureCard("🔐", tr("Locker Room"));
    featureCards["Shower"] = new FeatureCard("🚿", tr("Shower"));
    featureCards["Pro Shop"] = new FeatureCard("🛒", tr("Pro Shop"));
    featureCards["Coaching"] = new FeatureCard("👨‍🏫", tr("Coaching"));
    featureCards["Tournament Grade"] = new FeatureCard("🏆", tr("Tournament Grade"));
    featureCards["Heated"] = new FeatureCard("🔥", tr("Heated"));
    
    // Connect signals for all feature cards
    for (auto it = featureCards.begin(); it != featureCards.end(); ++it) {
        connect(it.value(), &FeatureCard::toggled, this, &AddCourtPage::onFeatureToggled);
    }
    
    // Add them to the grid layout
    int column = 0;
    int row = 0;
    const int COLS = 3;
    
    QList<QString> keys = featureCards.keys();
    for (int i = 0; i < keys.size(); i++) {
        column = i % COLS;
        row = i / COLS;
        featuresGridLayout->addWidget(featureCards[keys[i]], row, column);
    }
    
    featuresLayout->addWidget(featuresContainer);
    
    // Custom feature input with improved styling
    QHBoxLayout* customFeatureLayout = new QHBoxLayout();
    customFeatureLayout->setContentsMargins(0, 10, 0, 0);
    customFeatureLayout->setSpacing(12);
    
    featureEdit = new QLineEdit();
    featureEdit->setPlaceholderText(tr("Add custom feature..."));
    featureEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #374151;"
        "   border-radius: 10px;"
        "   padding: 12px 16px;"
        "   background-color: #1E293B;"
        "   color: #F9FAFB;"
        "   font-size: 15px;"
        "   min-height: 48px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #818CF8;"
        "   outline: none;"
        "}"
        "QLineEdit:hover {"
        "   border: 1px solid #4B5563;"
        "}"
        "QLineEdit::placeholder {"
        "   color: #9CA3AF;"
        "}"
    );
    
    addFeatureButton = new QPushButton(tr("Add"));
    addFeatureButton->setMinimumHeight(48);
    addFeatureButton->setCursor(Qt::PointingHandCursor);
    addFeatureButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #374151;"
        "   color: #D1D5DB;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 0 20px;"
        "   font-size: 15px;"
        "   font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4B5563;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #6B7280;"
        "}"
    );
    
    customFeatureLayout->addWidget(featureEdit);
    customFeatureLayout->addWidget(addFeatureButton);
    featuresLayout->addLayout(customFeatureLayout);
    
    QVBoxLayout* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    if (contentLayout) {
        contentLayout->insertWidget(1, featuresCard); // Insert after basic info card
    }
}

void AddCourtPage::setupTimeSlotButtons()
{
    // Create Time Slots card with modern styling
    QWidget* timeSlotsCard = new QWidget();
    timeSlotsCard->setObjectName("timeSlotsCard");
    timeSlotsCard->setStyleSheet(
        "QWidget#timeSlotsCard { "
        "   background-color: #1F2937; " // Default to dark theme
        "   border-radius: 16px; "
        "}"
    );
    
    QVBoxLayout* timeSlotsLayout = new QVBoxLayout(timeSlotsCard);
    timeSlotsLayout->setContentsMargins(28, 28, 28, 28);
    timeSlotsLayout->setSpacing(20);
    
    QLabel* timeSlotsTitle = new QLabel(tr("Available Time Slots"));
    timeSlotsTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #F9FAFB;");
    timeSlotsLayout->addWidget(timeSlotsTitle);
    
    QLabel* timeSlotsInfoLabel = new QLabel(tr("Select all time slots when this court is available for booking"));
    timeSlotsInfoLabel->setStyleSheet("color: #9CA3AF; font-size: 15px; margin-bottom: 8px;");
    timeSlotsLayout->addWidget(timeSlotsInfoLabel);
    
    // Time slots container with better organization
    timeSlotContainer = new QWidget();
    QGridLayout* timeSlotGridLayout = new QGridLayout(timeSlotContainer);
    timeSlotGridLayout->setContentsMargins(0, 0, 0, 0);
    timeSlotGridLayout->setVerticalSpacing(12);
    timeSlotGridLayout->setHorizontalSpacing(12);
    
    // Morning (8:00 - 12:00)
    QLabel* morningLabel = new QLabel(tr("Morning"));
    morningLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #9CA3AF; margin-top: 8px;");
    
    QWidget* morningContainer = new QWidget();
    QHBoxLayout* morningLayout = new QHBoxLayout(morningContainer);
    morningLayout->setContentsMargins(0, 0, 0, 0);
    morningLayout->setSpacing(10);
    morningLayout->setAlignment(Qt::AlignLeft);
    
    // Afternoon (12:00 - 17:00)
    QLabel* afternoonLabel = new QLabel(tr("Afternoon"));
    afternoonLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #9CA3AF; margin-top: 8px;");
    
    QWidget* afternoonContainer = new QWidget();
    QHBoxLayout* afternoonLayout = new QHBoxLayout(afternoonContainer);
    afternoonLayout->setContentsMargins(0, 0, 0, 0);
    afternoonLayout->setSpacing(10);
    afternoonLayout->setAlignment(Qt::AlignLeft);
    
    // Evening (17:00 - 22:00)
    QLabel* eveningLabel = new QLabel(tr("Evening"));
    eveningLabel->setStyleSheet("font-size: 15px; font-weight: 600; color: #9CA3AF; margin-top: 8px;");
    
    QWidget* eveningContainer = new QWidget();
    QHBoxLayout* eveningLayout = new QHBoxLayout(eveningContainer);
    eveningLayout->setContentsMargins(0, 0, 0, 0);
    eveningLayout->setSpacing(10);
    eveningLayout->setAlignment(Qt::AlignLeft);
    
    // Create time slot buttons
    timeSlotButtons.clear();
    
    // Generate time slots from 8:00 to 22:00 with 1-hour intervals
    QTime startTime(8, 0);
    QTime endTime(22, 0);
    
    while (startTime <= endTime) {
        QString timeText = startTime.toString("HH:mm");
        TimeSlotButton* button = new TimeSlotButton(timeText);
        
        // Set dark mode colors for time slot buttons
        button->setStyleSheet(
            "QPushButton {"
            "   background-color: #2D3748;"
            "   color: #D1D5DB;"
            "   border: none;"
            "   border-radius: 20px;"
            "   font-size: 14px;"
            "   font-weight: 500;"
            "   padding: 6px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #374151;"
            "}"
            "QPushButton:checked {"
            "   background-color: #818CF8;"
            "   color: #FFFFFF;"
            "}"
        );
        
        connect(button, &QPushButton::toggled, this, &AddCourtPage::onTimeSlotToggled);
        timeSlotButtons[timeText] = button;
        
        // Add to the appropriate container based on time
        if (startTime < QTime(12, 0)) {
            // Morning
            morningLayout->addWidget(button);
        } else if (startTime < QTime(17, 0)) {
            // Afternoon
            afternoonLayout->addWidget(button);
        } else {
            // Evening
            eveningLayout->addWidget(button);
        }
        
        startTime = startTime.addSecs(60 * 60); // Add 1 hour
    }
    
    // Add time section labels and containers to the grid
    timeSlotGridLayout->addWidget(morningLabel, 0, 0, 1, 1, Qt::AlignLeft);
    timeSlotGridLayout->addWidget(morningContainer, 1, 0);
    
    timeSlotGridLayout->addWidget(afternoonLabel, 2, 0, 1, 1, Qt::AlignLeft);
    timeSlotGridLayout->addWidget(afternoonContainer, 3, 0);
    
    timeSlotGridLayout->addWidget(eveningLabel, 4, 0, 1, 1, Qt::AlignLeft);
    timeSlotGridLayout->addWidget(eveningContainer, 5, 0);
    
    timeSlotsLayout->addWidget(timeSlotContainer);
    
    // Error label for time slots with improved styling
    timeSlotErrorLabel = new QLabel(tr("Please select at least one time slot"));
    timeSlotErrorLabel->setStyleSheet("color: #EF4444; font-size: 13px; margin-top: 4px;");
    timeSlotErrorLabel->setVisible(false);
    timeSlotsLayout->addWidget(timeSlotErrorLabel);
    
    QVBoxLayout* contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    if (contentLayout) {
        contentLayout->insertWidget(2, timeSlotsCard); // Insert after features card
    }
}

void AddCourtPage::validateField(QLineEdit* field, bool valid, const QString& errorMsg)
{
    // Use theme-aware styling
    QString bgColor = isDarkTheme ? "#1E293B" : "#FFFFFF";
    QString textColor = isDarkTheme ? "#F9FAFB" : "#1F2937";
    QString borderColorNormal = isDarkTheme ? "#374151" : "#E5E7EB";
    QString borderColorHover = isDarkTheme ? "#4B5563" : "#D1D5DB";
    QString borderColorFocus = isDarkTheme ? "#818CF8" : "#6366F1";
    QString errorColor = "#EF4444";
    
    if (valid) {
        field->setStyleSheet(QString(
            "QLineEdit {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;" 
            "}"
            "QLineEdit:focus {"
            "   border: 2px solid %4;"
            "   outline: none;"
            "}"
            "QLineEdit:hover {"
            "   border: 1px solid %5;"
            "}"
        ).arg(borderColorNormal).arg(bgColor).arg(textColor).arg(borderColorFocus).arg(borderColorHover));
        
        // Find and hide the error label
        QLayout* parentLayout = field->parentWidget()->layout();
        for (int i = 0; i < parentLayout->count(); ++i) {
            QWidget* widget = parentLayout->itemAt(i)->widget();
            if (QLabel* label = qobject_cast<QLabel*>(widget)) {
                if (label->styleSheet().contains("EF4444")) {
                    label->setVisible(false);
                    break;
                }
            }
        }
    } else {
        field->setStyleSheet(QString(
            "QLineEdit {"
            "   border: 1px solid %1;"
            "   border-radius: 8px;"
            "   padding: 8px 12px;"
            "   background-color: %2;"
            "   color: %3;"
            "   font-size: 13px;"
            "   min-height: 34px;"
            "}"
            "QLineEdit:focus {"
            "   border: 2px solid %1;"
            "   outline: none;"
            "}"
            "QLineEdit:hover {"
            "   border: 1px solid %1;"
            "}"
        ).arg(errorColor).arg(bgColor).arg(textColor));
        
        // Find and show the error label
        QLayout* parentLayout = field->parentWidget()->layout();
        for (int i = 0; i < parentLayout->count(); ++i) {
            QWidget* widget = parentLayout->itemAt(i)->widget();
            if (QLabel* label = qobject_cast<QLabel*>(widget)) {
                if (label->styleSheet().contains("EF4444")) {
                    if (!errorMsg.isEmpty()) {
                        label->setText(errorMsg);
                    }
                    label->setVisible(true);
                    break;
                }
            }
        }
    }
}

void AddCourtPage::onFeatureToggled(bool checked)
{
    FeatureCard* card = qobject_cast<FeatureCard*>(sender());
    if (card) {
        QString featureText = card->getText();
        
        if (checked) {
            if (!selectedFeatures.contains(featureText)) {
                selectedFeatures.append(featureText);
            }
        } else {
            selectedFeatures.removeAll(featureText);
        }
    }
}

void AddCourtPage::onTimeSlotToggled()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString timeSlot = button->text();
        if (button->isChecked()) {
            if (!selectedTimeSlots.contains(timeSlot)) {
                selectedTimeSlots.append(timeSlot);
            }
        } else {
            selectedTimeSlots.removeAll(timeSlot);
        }
        
        if (!selectedTimeSlots.isEmpty()) {
            timeSlotErrorLabel->setVisible(false);
        }
    }
}

void AddCourtPage::updateCharCount()
{
    int count = descriptionEdit->toPlainText().length();
    charCountLabel->setText(QString("%1/500").arg(count));
    
    if (count > 500) {
        charCountLabel->setStyleSheet("color: #EF4444; font-size: 13px;");
        
        // Truncate text to 500 characters
        QString truncatedText = descriptionEdit->toPlainText().left(500);
        descriptionEdit->blockSignals(true);
        descriptionEdit->setPlainText(truncatedText);
        descriptionEdit->blockSignals(false);
        
        charCountLabel->setText("500/500");
    } else {
        charCountLabel->setStyleSheet("color: #6B7280; font-size: 13px;");
    }
}

// Add optimized handler for court type toggle
void AddCourtPage::onCourtTypeToggled(bool checked)
{
    if (!checked) return;
    
    QRadioButton* radio = qobject_cast<QRadioButton*>(sender());
    if (!radio) return;
    
    if (radio == indoorRadio) {
    } else if (radio == outdoorRadio) {
    }
} 