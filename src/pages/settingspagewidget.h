#ifndef SETTINGSPAGEWIDGET_H
#define SETTINGSPAGEWIDGET_H

#include <QWidget>

class QComboBox;
class QLabel;
class QSpinBox;

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
    void setThemeStatus(const QString &text);
    void setDatabasePath(const QString &path);

signals:
    void refreshIntervalChanged(int ms);
    void themeModeChanged(const QString &themeMode);
    void applyRequested();
    void backupRequested();
    void restoreRequested();
    void cleanupRequested(int days);
    void reloadThemeRequested();

private:
    QComboBox *intervalCombo_ = nullptr;
    QComboBox *soundCombo_ = nullptr;
    QComboBox *themeModeCombo_ = nullptr;
    QLabel *themeStatusLabel_ = nullptr;
    QLabel *dbPathLabel_ = nullptr;
    QSpinBox *cleanupDaysSpin_ = nullptr;
};

#endif // SETTINGSPAGEWIDGET_H