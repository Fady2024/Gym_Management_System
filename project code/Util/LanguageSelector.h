#ifndef LANGUAGESELECTOR_H
#define LANGUAGESELECTOR_H

#include <QComboBox>

class LanguageSelector : public QComboBox
{
    Q_OBJECT

public:
    explicit LanguageSelector(QWidget* parent = nullptr);
    void updateTheme(bool isDark);

private slots:
    void onLanguageChanged(int index);

private:
    void setupUI();
    bool isDarkTheme;
};

#endif // LANGUAGESELECTOR_H 