#include "addmemberpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QImageReader>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpression>

AddMemberPage::AddMemberPage(UserDataManager* userDataManager, MemberDataManager* memberDataManger, QWidget* parent)
    : QMainWindow(parent)
    , isDarkTheme(ThemeManager::getInstance().isDarkTheme())
    , userDataManager(userDataManager)
    , memberDataManger(memberDataManger)
    , opacityEffect(new QGraphicsOpacityEffect(this))
    , messageWidget(nullptr)
    , messageText(nullptr)
    , messageIcon(nullptr)
    , messageTimer(nullptr)
{

    selectedImagePath.clear();

    setMinimumSize(800, 600);

    setupUI();
    setupGlassEffect();

    QTimer::singleShot(250, this, &AddMemberPage::setupMessageWidget);

    updateTheme(isDarkTheme);

    QTimer::singleShot(50, this, [this]() {
    connect(&ThemeManager::getInstance(), &ThemeManager::themeChanged,
        this, [this](bool isDark) {
            isDarkTheme = isDark;
            updateTheme(isDark);
        });

    connect(&LanguageManager::getInstance(), &LanguageManager::languageChanged,
        this, &AddMemberPage::retranslateUI);
    });
}

void AddMemberPage::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    auto* contentWidget = new QWidget;
    contentWidget->setObjectName("contentWidget");
    auto* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setAlignment(Qt::AlignCenter);
    contentLayout->setContentsMargins(24, 24, 24, 24);

    auto* formContainer = new QWidget;
    formContainer->setObjectName("formContainer");
    formContainer->setStyleSheet(UIUtils::getAuthContainerStyle(isDarkTheme));
    formContainer->setFixedWidth(400);

    auto* formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(24, 24, 24, 24);
    formLayout->setAlignment(Qt::AlignCenter);

    auto* titleLabel2 = new QLabel(tr("Add New Member"));
    titleLabel2->setObjectName("titleLabel");
    titleLabel2->setStyleSheet(UIUtils::getWelcomeLabelStyle(isDarkTheme));
    titleLabel2->setTextFormat(Qt::RichText);
    titleLabel2->setAlignment(Qt::AlignCenter);

    auto* profileContainer = new QWidget;
    profileContainer->setObjectName("profileContainer");
    auto* profileLayout = new QVBoxLayout(profileContainer);
    profileLayout->setSpacing(8);
    profileLayout->setContentsMargins(0, 8, 0, 8);
    profileLayout->setAlignment(Qt::AlignCenter);

    profileContainer->setStyleSheet(
        "QWidget#profileContainer {"
        "   background: transparent;"
        "   border: none;"
        "}"
    );

    profileImageButton = new QPushButton;
    profileImageButton->setObjectName("profileImageButton");
    profileImageButton->setFixedSize(100, 100);
    profileImageButton->setIcon(UIUtils::getIcon("person.png", 50));
    profileImageButton->setIconSize(QSize(50, 50));

    profileImageButton->setFocusPolicy(Qt::NoFocus);
    profileImageButton->setAttribute(Qt::WA_MacShowFocusRect, false);

    profileImageButton->setStyleSheet(
        "QPushButton#profileImageButton {"
        "   background-color: transparent;"
        "   border: 2px solid " + QString(isDarkTheme ? "#475569" : "#cbd5e1") + ";"
        "   border-radius: 50px;"
        "   padding: 0;"
        "   outline: none;"
        "}"
        "QPushButton#profileImageButton:hover {"
        "   border: 2px solid #8B5CF6;"
        "}"
        "QPushButton#profileImageButton:focus {"
        "   outline: none;"
        "   border: 2px solid " + QString(isDarkTheme ? "#475569" : "#cbd5e1") + ";"
        "}"
        "QPushButton#profileImageButton:pressed {"
        "   border: 2px solid #8B5CF6;"
        "}"
    );

    connect(profileImageButton, &QPushButton::clicked, this, &AddMemberPage::selectProfileImage);

    auto* uploadLabel = new QLabel(tr("Upload Photo"));
    uploadLabel->setStyleSheet(UIUtils::getProfileUploadLabelStyle(isDarkTheme));
    uploadLabel->setAlignment(Qt::AlignCenter);

    profileLayout->addWidget(profileImageButton, 0, Qt::AlignCenter);
    profileLayout->addWidget(uploadLabel, 0, Qt::AlignCenter);

    nameInput = new QLineEdit;
    nameInput->setPlaceholderText(tr("Full Name"));
    nameInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    nameInput->setFixedHeight(45);
    nameInput->setFixedWidth(352);

    auto* nameIcon = new QLabel(nameInput);
    nameIcon->setPixmap(UIUtils::getIconWithColor("person_bw.png", QColor(0x8B5CF6), 18));
    nameIcon->setGeometry(16, 14, 18, 18);

    emailInput = new QLineEdit;
    emailInput->setPlaceholderText(tr("Email Address"));
    emailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    emailInput->setFixedHeight(45);
    emailInput->setFixedWidth(352);

    auto* emailIcon = new QLabel(emailInput);
    emailIcon->setPixmap(UIUtils::getIconWithColor("mail.png", QColor(0x8B5CF6), 18));
    emailIcon->setGeometry(16, 14, 18, 18);

    passwordInput = new QLineEdit;
    passwordInput->setPlaceholderText(tr("Password"));
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    passwordInput->setFixedHeight(45);
    passwordInput->setFixedWidth(352);

    auto* lockIcon = new QLabel(passwordInput);
    lockIcon->setPixmap(UIUtils::getIconWithColor("lock.png", QColor(0x8B5CF6), 18));
    lockIcon->setGeometry(16, 14, 18, 18);

    auto* togglePasswordIcon = new QLabel(passwordInput);
    togglePasswordIcon->setPixmap(UIUtils::getIconWithColor("close eyes.png", QColor(0x8B5CF6), 18));
    togglePasswordIcon->setStyleSheet("QLabel { cursor: pointer; }");
    togglePasswordIcon->setGeometry(352 - 34, 14, 18, 18);
    togglePasswordIcon->installEventFilter(this);

    dateOfBirthInput = new QDateEdit;
    dateOfBirthInput->setDisplayFormat("dd/MM/yyyy");
    dateOfBirthInput->setCalendarPopup(true);
    dateOfBirthInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    dateOfBirthInput->setFixedHeight(45);
    dateOfBirthInput->setFixedWidth(352);

    auto* calendarIcon = new QLabel(dateOfBirthInput);
    calendarIcon->setPixmap(UIUtils::getIconWithColor("calendar.png", QColor(0x8B5CF6), 18));
    calendarIcon->setGeometry(16, 14, 18, 18);

    addMemberButton = new QPushButton(tr("Add Member"));
    addMemberButton->setFixedHeight(45);
    addMemberButton->setFixedWidth(352);
    addMemberButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));
    connect(addMemberButton, &QPushButton::clicked, this, &AddMemberPage::handleAddMember);

    formLayout->addWidget(titleLabel2);
    formLayout->addWidget(profileContainer, 0, Qt::AlignCenter);
    formLayout->addWidget(nameInput, 0, Qt::AlignCenter);
    formLayout->addWidget(emailInput, 0, Qt::AlignCenter);
    formLayout->addWidget(passwordInput, 0, Qt::AlignCenter);
    formLayout->addWidget(dateOfBirthInput, 0, Qt::AlignCenter);
    formLayout->addWidget(addMemberButton, 0, Qt::AlignCenter);

    contentLayout->addStretch();
    contentLayout->addWidget(formContainer, 0, Qt::AlignCenter);
    contentLayout->addStretch();
    mainLayout->addWidget(contentWidget);

    setCentralWidget(centralWidget);
}

void AddMemberPage::handleAddMember()
{
    if (!userDataManager || !memberDataManger) {
        showError(tr("System error. Please try again later."));
        return;
    }

    try {

        const QString name = nameInput ? nameInput->text() : QString();
        if (name.isEmpty()) {
            showError(tr("Please enter member name"));
            if (nameInput) nameInput->setFocus();
            return;
        }

        QString errorMessage;
        if (!userDataManager->validateName(name, errorMessage)) {
            showError(tr("Name: %1").arg(errorMessage));
            if (nameInput) nameInput->setFocus();
            return;
        }

        const QString email = emailInput ? emailInput->text() : QString();
        if (email.isEmpty()) {
            showError(tr("Please enter member email"));
            if (emailInput) emailInput->setFocus();
            return;
        }

        if (!userDataManager->validateEmail(email, errorMessage)) {
            showError(tr("Email: %1").arg(errorMessage));
            if (emailInput) emailInput->setFocus();
            return;
        }

        if (userDataManager->emailExists(email)) {
            showError(tr("Email is already registered. Please use a different email."));
            if (emailInput) emailInput->setFocus();
            return;
        }

        const QString password = passwordInput ? passwordInput->text() : QString();
        if (password.isEmpty()) {
            showError(tr("Please enter a password"));
            if (passwordInput) passwordInput->setFocus();
            return;
        }

        if (!userDataManager->validatePassword(password, errorMessage)) {
            showError(tr("Password: %1").arg(errorMessage));
            if (passwordInput) passwordInput->setFocus();
            return;
        }

        QDate dateOfBirth = dateOfBirthInput ? dateOfBirthInput->date() : QDate();
        if (!dateOfBirth.isValid()) {
            showError(tr("Please enter a valid date of birth"));
            if (dateOfBirthInput) dateOfBirthInput->setFocus();
            return;
        }

        if (!userDataManager->validateDateOfBirth(dateOfBirth, errorMessage)) {
            showError(tr("Date of birth: %1").arg(errorMessage));
            if (dateOfBirthInput) dateOfBirthInput->setFocus();
            return;
        }

        User newUser(name, email, password, selectedImagePath, dateOfBirth);
        if (!userDataManager->saveUserData(newUser, errorMessage)) {
            showError(tr("Failed to create user account: %1").arg(errorMessage));
            return;
        }

        User createdUser = userDataManager->getUserData(email);
        if (createdUser.getId() == 0) {
            showError(tr("Failed to retrieve user data after creation"));
            return;
        }

        if (!memberDataManger->createMemberFromUser(createdUser, errorMessage)) {
            showError(tr("Failed to create member: %1").arg(errorMessage));
            return;
        }

        showSuccess(tr("Member added successfully!"));

        const auto delayTimer = new QTimer(this);
        delayTimer->setSingleShot(true);
        connect(delayTimer, &QTimer::timeout, this, [this, email]() {
            clearFields();
            emit memberAdded(email);
        });
        delayTimer->start(1500);

    } catch (const std::exception& e) {
        qDebug() << "Exception in handleAddMember: " << e.what();
        showError(tr("An error occurred while adding member. Please try again."));
    } catch (...) {
        qDebug() << "Unknown exception in handleAddMember";
        showError(tr("An unexpected error occurred. Please try again."));
    }
}

void AddMemberPage::selectProfileImage()
{
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Profile Image",
        "",
        "Images (*.png *.jpg *.jpeg *.bmp *.gif *.tiff *.webp *.svg *.ico)"
    );

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.exists()) {
            showError(tr("File does not exist: %1").arg(filePath));
            return;
        }

        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();
        QString baseName = fileInfo.baseName();

        QImage image;
        bool loaded = false;

        if (extension == "jpg" || extension == "jpeg") {
            QImageReader reader(filePath);
            reader.setFormat("jpeg");
            reader.setAutoTransform(true);
            reader.setDecideFormatFromContent(true);
            image = reader.read();
            loaded = !image.isNull();

            if (!loaded) {
                qDebug() << "JPEG loading error:" << reader.errorString();
            }
        }

        if (!loaded) {
            image = QImage(filePath);
            loaded = !image.isNull();
        }

        if (!loaded) {
            QImageReader reader(filePath);
            QByteArray format = QImageReader::imageFormat(filePath);
            if (!format.isEmpty()) {
                reader.setFormat(format);
            }
            reader.setAutoTransform(true);
            reader.setDecideFormatFromContent(true);
            image = reader.read();
            loaded = !image.isNull();

            if (!loaded) {
                qDebug() << "QImageReader error:" << reader.errorString();
            }
        }

        if (!loaded) {
            QPixmap pixmap(filePath);
            if (!pixmap.isNull()) {
                image = pixmap.toImage();
                loaded = !image.isNull();
            }
        }

        if (!loaded) {
            if (extension == "jpg" || extension == "jpeg") {
                showError(tr("Failed to load JPEG image: %1\nPlease ensure the file is not corrupted and try again.").arg(filePath));
            } else {
                showError(tr("Failed to load image: %1\nSupported formats are PNG, JPEG, BMP, and GIF.").arg(filePath));
            }
            return;
        }

        QPixmap uploadedPhoto = QPixmap::fromImage(image);

        if (uploadedPhoto.isNull()) {
            showError(tr("Failed to convert image: %1").arg(filePath));
            return;
        }

        int size = profileImageButton->width();
        QPixmap circularPhoto(size, size);
        circularPhoto.fill(Qt::transparent);

        QPainter painter(&circularPhoto);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QPainterPath path;
        path.addEllipse(0, 0, size, size);
        painter.setClipPath(path);

        QPixmap squarePhoto;
        if (uploadedPhoto.width() != uploadedPhoto.height()) {
            int minDim = qMin(uploadedPhoto.width(), uploadedPhoto.height());
            QRect cropRect;

            if (uploadedPhoto.width() > uploadedPhoto.height()) {
                cropRect = QRect((uploadedPhoto.width() - minDim) / 2, 0, minDim, minDim);
            } else {
                cropRect = QRect(0, (uploadedPhoto.height() - minDim) / 2, minDim, minDim);
            }

            squarePhoto = uploadedPhoto.copy(cropRect);
        } else {
            squarePhoto = uploadedPhoto;
        }

        const QPixmap scaledPhoto = squarePhoto.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        painter.drawPixmap(0, 0, scaledPhoto);

        painter.setClipping(false);
        painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
        painter.drawEllipse(1, 1, size-2, size-2);

        profileImageButton->setIcon(QIcon(circularPhoto));
        profileImageButton->setIconSize(QSize(size, size));

        lastCircularPhoto = circularPhoto;

        QString projectDir;

#ifdef FORCE_SOURCE_DIR
        projectDir = QString::fromUtf8(SOURCE_DATA_DIR);
        qDebug() << "AddMember - Using source directory path:" << projectDir;
#else
        projectDir = QCoreApplication::applicationDirPath();
        projectDir = QFileInfo(projectDir).dir().absolutePath();
        qDebug() << "AddMember - Using application directory path:" << projectDir;
#endif

        QString usersPhotoDir = projectDir + "/project code/UsersPhoto";
        qDebug() << "AddMember - Users photo directory path:" << usersPhotoDir;

        QDir dir(usersPhotoDir);
        if (!dir.exists() && !dir.mkpath(".")) {
            showError(tr("Failed to create directory: %1").arg(dir.absolutePath()));
            return;
        }

        QString newFileName = baseName + "." + extension;
        QString newAbsolutePath = dir.absoluteFilePath(newFileName);

        if (QFile::exists(newAbsolutePath)) {
            QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            newFileName = baseName + "_" + timestamp + "." + extension;
            newAbsolutePath = dir.absoluteFilePath(newFileName);
        }

        if (!image.save(newAbsolutePath)) {
            showError(tr("Failed to save photo to: %1").arg(newAbsolutePath));
            return;
        }

        selectedImagePath = "project code/UsersPhoto/" + newFileName;
        showSuccess(tr("Photo uploaded successfully!"));
    }
}

void AddMemberPage::clearFields()
{
    nameInput->clear();
    emailInput->clear();
    passwordInput->clear();
    dateOfBirthInput->setDate(QDate::currentDate());
    selectedImagePath.clear();

    int size = profileImageButton->width();
    QPixmap defaultPhoto(size, size);
    defaultPhoto.fill(Qt::transparent);

    QPainter painter(&defaultPhoto);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(isDarkTheme ? "#1e293b" : "#f1f5f9"));
    painter.drawEllipse(0, 0, size, size);

    painter.setClipping(false);
    painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
    painter.drawEllipse(1, 1, size-2, size-2);

    const QPixmap defaultIcon = UIUtils::getIcon("person.png", size/2);
    if (!defaultIcon.isNull()) {
        painter.drawPixmap(size/4, size/4, defaultIcon);
    }

    profileImageButton->setIcon(QIcon(defaultPhoto));
    profileImageButton->setIconSize(QSize(size, size));
    lastCircularPhoto = QPixmap();
}

void AddMemberPage::showError(const QString& message)
{
    if (messageText) {
        messageText->setText(message);
        animateMessageWidget(true);
    } else {
        QMessageBox::critical(this, tr("Error"), message);
    }
}

void AddMemberPage::showSuccess(const QString& message)
{
    if (messageText) {
        messageText->setText(message);
        animateMessageWidget(false);
    } else {
        QMessageBox::information(this, tr("Success"), message);
    }
}

void AddMemberPage::setupGlassEffect()
{
    if (const auto formContainer = findChild<QWidget*>("formContainer")) {
        formContainer->setGraphicsEffect(opacityEffect);
        opacityEffect->setOpacity(0.95);
    }
}

void AddMemberPage::setupMessageWidget()
{
    messageWidget = new QWidget(this);
    messageWidget->setObjectName("messageWidget");
    messageWidget->setMinimumHeight(48);
    messageWidget->setMaximumWidth(600);
    messageWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    messageWidget->setAttribute(Qt::WA_TranslucentBackground);
    messageWidget->hide();

    const auto messageLayout = new QHBoxLayout(messageWidget);
    messageLayout->setContentsMargins(16, 12, 16, 12);
    messageLayout->setSpacing(12);
    messageLayout->setAlignment(Qt::AlignCenter);

    const auto iconContainer = new QWidget;
    const auto iconLayout = new QVBoxLayout(iconContainer);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->setSpacing(0);
    iconLayout->setAlignment(Qt::AlignCenter);

    messageIcon = new QLabel;
    messageIcon->setFixedSize(24, 24);
    messageIcon->setStyleSheet(UIUtils::getMessageIconStyle(isDarkTheme));
    messageIcon->setAlignment(Qt::AlignCenter);
    iconLayout->addWidget(messageIcon);

    const auto textContainer = new QWidget;
    const auto textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(0);
    textLayout->setAlignment(Qt::AlignCenter);

    messageText = new QLabel;
    messageText->setObjectName("messageText");
    messageText->setWordWrap(true);
    messageText->setAlignment(Qt::AlignCenter);
    messageText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    messageText->setMinimumWidth(250);
    textLayout->addWidget(messageText);

    messageLayout->addWidget(iconContainer);
    messageLayout->addWidget(textContainer, 1);

    messageWidget->move(width() / 2 - messageWidget->width() / 2, 100);

    messageTimer = new QTimer(this);
    messageTimer->setSingleShot(true);
    connect(messageTimer, &QTimer::timeout, messageWidget, &QWidget::hide);
}

void AddMemberPage::animateMessageWidget(bool isError)
{
    if (!messageWidget || !messageText || !messageIcon) return;

    messageWidget->setStyleSheet(UIUtils::getMessageWidgetStyle(isDarkTheme, isError));

    const QPixmap icon = UIUtils::getIcon(isError ? "error.png" : "success.png", 24);
    if (!icon.isNull()) {
        messageIcon->setPixmap(icon);
    } else {
        qDebug() << "Failed to load" << (isError ? "error" : "success") << "icon";
        messageIcon->setText(isError ? "X" : "✓");
    }

    messageWidget->adjustSize();
    messageWidget->move(width() / 2 - messageWidget->width() / 2, 100);
    messageWidget->show();

    messageTimer->start(5000);
}

void AddMemberPage::updateTheme(bool isDark)
{
    isDarkTheme = isDark;

    const QString baseStyle = isDark ?
        "QMainWindow { background-color: #1A1F2C; }" :
        "QMainWindow { background-color: #F9FAFB; }";
    setStyleSheet(baseStyle);

    if (const auto formContainer = findChild<QWidget*>("formContainer")) {
        formContainer->setStyleSheet(UIUtils::getAuthContainerStyle(isDarkTheme));
    }

    if (messageWidget) {
        messageWidget->setStyleSheet(UIUtils::getMessageWidgetStyle(isDarkTheme, false));
    }
    if (messageIcon) {
        messageIcon->setStyleSheet(UIUtils::getMessageIconStyle(isDarkTheme));
    }

    if (profileImageButton) {

        profileImageButton->setStyleSheet(
            "QPushButton#profileImageButton {"
            "   background-color: transparent;"
            "   border: 2px solid " + QString(isDarkTheme ? "#475569" : "#cbd5e1") + ";"
            "   border-radius: " + QString::number(profileImageButton->width() / 2) + "px;"
            "   padding: 0;"
            "   outline: none;"
            "}"
            "QPushButton#profileImageButton:hover {"
            "   border: 2px solid #8B5CF6;"
            "}"
            "QPushButton#profileImageButton:focus {"
            "   outline: none;"
            "   border: 2px solid " + QString(isDarkTheme ? "#475569" : "#cbd5e1") + ";"
            "}"
            "QPushButton#profileImageButton:pressed {"
            "   border: 2px solid #8B5CF6;"
            "}"
        );

        if (!selectedImagePath.isEmpty() && !lastCircularPhoto.isNull()) {
            const int size = profileImageButton->width();
            QPixmap circularPhoto(size, size);
            circularPhoto.fill(Qt::transparent);

            QPainter painter(&circularPhoto);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);

            QPainterPath path;
            path.addEllipse(0, 0, size, size);
            painter.setClipPath(path);

            painter.drawPixmap(0, 0, lastCircularPhoto.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

            painter.setClipping(false);
            painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
            painter.drawEllipse(1, 1, size-2, size-2);

            profileImageButton->setIcon(QIcon(circularPhoto));
            profileImageButton->setIconSize(QSize(size, size));
        }
    }

    if (nameInput) nameInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (emailInput) emailInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (passwordInput) passwordInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    if (dateOfBirthInput) dateOfBirthInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));

    if (addMemberButton) addMemberButton->setStyleSheet(UIUtils::getButtonStyle(isDarkTheme));

    updateAllTextColors();
}

void AddMemberPage::updateAllTextColors()
{
    const QString titleStyle = isDarkTheme ?
        "color: #F9FAFB; font-size: 24px; font-weight: 600;" :
        "color: #111827; font-size: 24px; font-weight: 600;";

    const QString labelStyle = isDarkTheme ?
        "color: #E5E7EB; font-size: 14px;" :
        "color: #4B5563; font-size: 14px;";

    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->text() == tr("Add New Member")) {
            label->setStyleSheet(UIUtils::getWelcomeLabelStyle(isDarkTheme));
        } else if (label->text() == tr("Upload Photo")) {
            label->setStyleSheet(UIUtils::getProfileUploadLabelStyle(isDarkTheme));
        } else {
            label->setStyleSheet(labelStyle);
        }
    }

    QList<QLineEdit*> inputs = findChildren<QLineEdit*>();
    for (QLineEdit* input : inputs) {
        input->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    }

    if (dateOfBirthInput) {
        dateOfBirthInput->setStyleSheet(UIUtils::getInputStyle(isDarkTheme));
    }
}

void AddMemberPage::updateLayout()
{
    const QSize size = this->size();

    if (const auto formContainer = findChild<QWidget*>("formContainer")) {
        int containerWidth = 400;

        if (size.width() < 600) {
            containerWidth = qMax(280, static_cast<int>(size.width() * 0.9));
        } else if (size.width() < 800) {
            containerWidth = qMax(350, static_cast<int>(size.width() * 0.7));
        } else {
            containerWidth = qMin(400, static_cast<int>(size.width() * 0.5));
        }

        formContainer->setFixedWidth(containerWidth);

        if (QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(formContainer->layout())) {
            int margin = containerWidth < 350 ? 16 : 24;
            int spacing = containerWidth < 350 ? 12 : 16;
            layout->setContentsMargins(margin, margin, margin, margin);
            layout->setSpacing(spacing);
        }
    }

    if (profileImageButton) {

        int profileSize = qMin(qMax(size.width() / 10, 80), 140);
        profileImageButton->setFixedSize(profileSize, profileSize);

        QString currentStyle = profileImageButton->styleSheet();
        currentStyle.replace(QRegularExpression("border-radius:\\s*\\d+px"),
                            QString("border-radius: %1px").arg(profileSize / 2));
        profileImageButton->setStyleSheet(currentStyle);

        if (!selectedImagePath.isEmpty() && !lastCircularPhoto.isNull()) {
            QPixmap scaledPhoto = lastCircularPhoto.scaled(profileSize, profileSize,
                Qt::KeepAspectRatio, Qt::SmoothTransformation);
            profileImageButton->setIcon(QIcon(scaledPhoto));
            profileImageButton->setIconSize(QSize(profileSize, profileSize));
        } else {

            QPixmap defaultPhoto(profileSize, profileSize);
            defaultPhoto.fill(Qt::transparent);

            QPainter painter(&defaultPhoto);
            painter.setRenderHint(QPainter::Antialiasing);

            QPainterPath path;
            path.addEllipse(0, 0, profileSize, profileSize);
            painter.setClipPath(path);

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(isDarkTheme ? "#1e293b" : "#f1f5f9"));
            painter.drawEllipse(0, 0, profileSize, profileSize);

            painter.setClipping(false);
            painter.setPen(QPen(QColor(isDarkTheme ? "#475569" : "#cbd5e1"), 2));
            painter.drawEllipse(1, 1, profileSize-2, profileSize-2);

            const QPixmap defaultIcon = UIUtils::getIcon("person.png", profileSize/2);
            if (!defaultIcon.isNull()) {
                painter.drawPixmap(profileSize/4, profileSize/4, defaultIcon);
            }

            profileImageButton->setIcon(QIcon(defaultPhoto));
            profileImageButton->setIconSize(QSize(profileSize, profileSize));
        }
    }

    int inputWidth = qMin(352, static_cast<int>(size.width() * 0.7));

    if (const auto formContainer = findChild<QWidget*>("formContainer")) {
        inputWidth = qMin(inputWidth, formContainer->width() - 48);
    }

    if (nameInput) nameInput->setFixedWidth(inputWidth);
    if (emailInput) emailInput->setFixedWidth(inputWidth);
    if (passwordInput) passwordInput->setFixedWidth(inputWidth);
    if (dateOfBirthInput) dateOfBirthInput->setFixedWidth(inputWidth);
    if (addMemberButton) addMemberButton->setFixedWidth(inputWidth);

    int inputHeight = size.height() < 600 ? 40 : 45;
    if (nameInput) nameInput->setFixedHeight(inputHeight);
    if (emailInput) emailInput->setFixedHeight(inputHeight);
    if (passwordInput) passwordInput->setFixedHeight(inputHeight);
    if (dateOfBirthInput) dateOfBirthInput->setFixedHeight(inputHeight);

    if (addMemberButton) {
        addMemberButton->setFixedHeight(inputHeight + 5);
    }

    if (messageWidget) {
        messageWidget->adjustSize();
        int yPos = size.height() < 700 ? 80 : 100;
        messageWidget->move(width() / 2 - messageWidget->width() / 2, yPos);
    }

    if (const auto contentWidget = findChild<QWidget*>("contentWidget")) {
        if (QLayout* layout = contentWidget->layout()) {
            layout->invalidate();
            layout->activate();
        }
        contentWidget->updateGeometry();
    }

    setMinimumSize(qMax(480, size.width()), qMax(500, size.height()));
}

void AddMemberPage::retranslateUI()
{
    QList<QLabel*> labels = findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->objectName() == "titleLabel") {
            label->setText(tr("Add New Member"));
        } else if (label->text().contains("Upload")) {
            label->setText(tr("Upload Photo"));
        }
    }

    if (nameInput) nameInput->setPlaceholderText(tr("Full Name"));
    if (emailInput) emailInput->setPlaceholderText(tr("Email Address"));
    if (passwordInput) passwordInput->setPlaceholderText(tr("Password"));

    if (dateOfBirthInput) dateOfBirthInput->setDisplayFormat(tr("dd/MM/yyyy"));

    if (addMemberButton) addMemberButton->setText(tr("Add Member"));
}