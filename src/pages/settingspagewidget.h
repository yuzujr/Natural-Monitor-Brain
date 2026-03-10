#ifndef SETTINGSPAGEWIDGET_H
#define SETTINGSPAGEWIDGET_H

#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QString;

class SettingsPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPageWidget(QWidget *parent = nullptr);

    void setRefreshInterval(int ms);
    void setSoundMode(const QString &soundMode);
    QString soundMode() const;
    void setThemeMode(const QString &themeMode);
    QString themeMode() const;
    void setDatabasePath(const QString &path);
    void setLanguageKey(const QString &languageKey);
    QString languageKey() const;

signals:
    void refreshIntervalChanged(int ms);
    void themeModeChanged(const QString &themeMode);
    void applyRequested();
    void backupRequested();
    void restoreRequested();
    void cleanupRequested(int days);
    void reloadThemeRequested();
    void languageChanged(const QString &languageKey);

private:
    QComboBox *intervalCombo_ = nullptr;
    QComboBox *soundCombo_ = nullptr;
    QComboBox *themeModeCombo_ = nullptr;
    QLabel *dbPathLabel_ = nullptr;
    QComboBox *languageCombo_ = nullptr;
    QSpinBox *cleanupDaysSpin_ = nullptr;
};

#endif // SETTINGSPAGEWIDGET_H