#include "settingspagewidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include "common/uistyles.h"

SettingsPageWidget::SettingsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    auto *introLabel = new QLabel(tr("统一管理刷新频率、提示方式和数据库维护操作，设置会在下次启动时自动生效。"), this);
    introLabel->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(introLabel);

    auto *settingsGroup = new QGroupBox(tr("系统设置"), this);
    auto *settingsLayout = new QGridLayout(settingsGroup);

    intervalCombo_ = new QComboBox(settingsGroup);
    intervalCombo_->addItem(tr("1 秒"), 1000);
    intervalCombo_->addItem(tr("5 秒"), 5000);
    intervalCombo_->addItem(tr("10 秒"), 10000);
    connect(intervalCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                emit refreshIntervalChanged(intervalCombo_->itemData(index).toInt());
            });

    soundCombo_ = new QComboBox(settingsGroup);
    soundCombo_->addItem(tr("提示音"), QStringLiteral("beep"));
    soundCombo_->addItem(tr("静音"), QStringLiteral("mute"));

    themeModeCombo_ = new QComboBox(settingsGroup);
    themeModeCombo_->addItem(tr("跟随系统"), QStringLiteral("system"));
    themeModeCombo_->addItem(tr("浅色"), QStringLiteral("light"));
    themeModeCombo_->addItem(tr("深色"), QStringLiteral("dark"));
    connect(themeModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                emit themeModeChanged(themeModeCombo_->itemData(index).toString());
            });

    settingsLayout->addWidget(new QLabel(tr("刷新频率"), settingsGroup), 0, 0);
    settingsLayout->addWidget(intervalCombo_, 0, 1);
    settingsLayout->addWidget(new QLabel(tr("主题模式"), settingsGroup), 1, 0);
    settingsLayout->addWidget(themeModeCombo_, 1, 1);
    settingsLayout->addWidget(new QLabel(tr("主题状态"), settingsGroup), 2, 0);

    themeStatusLabel_ = new QLabel(tr("跟随系统默认主题"), settingsGroup);
    themeStatusLabel_->setStyleSheet(UiStyles::secondaryTextStyle());
    settingsLayout->addWidget(themeStatusLabel_, 2, 1);

    auto *reloadThemeButton = new QPushButton(tr("重新加载主题"), settingsGroup);
    UiStyles::applyButtonVariant(reloadThemeButton, QStringLiteral("secondary"));
    connect(reloadThemeButton, &QPushButton::clicked, this, &SettingsPageWidget::reloadThemeRequested);
    settingsLayout->addWidget(reloadThemeButton, 2, 2);

    settingsLayout->addWidget(new QLabel(tr("报警提示"), settingsGroup), 3, 0);
    settingsLayout->addWidget(soundCombo_, 3, 1);

    auto *applyButton = new QPushButton(tr("应用设置"), settingsGroup);
    UiStyles::applyButtonVariant(applyButton, QStringLiteral("primary"));
    connect(applyButton, &QPushButton::clicked, this, &SettingsPageWidget::applyRequested);
    settingsLayout->addWidget(applyButton, 4, 0, 1, 3);
    layout->addWidget(settingsGroup);

    auto *dbGroup = new QGroupBox(tr("数据库维护"), this);
    auto *dbLayout = new QGridLayout(dbGroup);

    dbPathLabel_ = new QLabel(dbGroup);
    dbPathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dbPathLabel_->setWordWrap(true);

    auto *backupButton = new QPushButton(tr("备份数据库"), dbGroup);
    auto *restoreButton = new QPushButton(tr("恢复数据库"), dbGroup);
    UiStyles::applyButtonVariant(backupButton, QStringLiteral("primary"));
    UiStyles::applyButtonVariant(restoreButton, QStringLiteral("secondary"));
    connect(backupButton, &QPushButton::clicked, this, &SettingsPageWidget::backupRequested);
    connect(restoreButton, &QPushButton::clicked, this, &SettingsPageWidget::restoreRequested);

    cleanupDaysSpin_ = new QSpinBox(dbGroup);
    cleanupDaysSpin_->setRange(1, 365);
    cleanupDaysSpin_->setValue(30);
    cleanupDaysSpin_->setSuffix(tr(" 天"));

    auto *cleanupButton = new QPushButton(tr("清理历史数据"), dbGroup);
    UiStyles::applyButtonVariant(cleanupButton, QStringLiteral("danger"));
    connect(cleanupButton, &QPushButton::clicked, this, [this]() {
        emit cleanupRequested(cleanupDaysSpin_->value());
    });

    dbLayout->addWidget(new QLabel(tr("数据库路径"), dbGroup), 0, 0);
    dbLayout->addWidget(dbPathLabel_, 0, 1, 1, 3);
    dbLayout->addWidget(backupButton, 1, 0);
    dbLayout->addWidget(restoreButton, 1, 1);
    dbLayout->addWidget(new QLabel(tr("保留周期"), dbGroup), 2, 0);
    dbLayout->addWidget(cleanupDaysSpin_, 2, 1);
    dbLayout->addWidget(cleanupButton, 2, 2);

    layout->addWidget(dbGroup);
    layout->addStretch();
}

void SettingsPageWidget::setRefreshInterval(int ms)
{
    const int index = intervalCombo_->findData(ms);
    if (index >= 0 && intervalCombo_->currentIndex() != index) {
        QSignalBlocker blocker(intervalCombo_);
        intervalCombo_->setCurrentIndex(index);
    }
}

void SettingsPageWidget::setSoundMode(const QString &soundMode)
{
    const int index = soundCombo_->findData(soundMode);
    if (index >= 0 && soundCombo_->currentIndex() != index) {
        QSignalBlocker blocker(soundCombo_);
        soundCombo_->setCurrentIndex(index);
    }
}

QString SettingsPageWidget::soundMode() const
{
    return soundCombo_->currentData().toString();
}

void SettingsPageWidget::setThemeMode(const QString &themeMode)
{
    const int index = themeModeCombo_->findData(themeMode);
    if (index >= 0 && themeModeCombo_->currentIndex() != index) {
        QSignalBlocker blocker(themeModeCombo_);
        themeModeCombo_->setCurrentIndex(index);
    }
}

QString SettingsPageWidget::themeMode() const
{
    return themeModeCombo_->currentData().toString();
}

void SettingsPageWidget::setThemeStatus(const QString &text)
{
    themeStatusLabel_->setText(text);
}

void SettingsPageWidget::setDatabasePath(const QString &path)
{
    dbPathLabel_->setText(path);
}