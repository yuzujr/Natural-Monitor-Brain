#ifndef THEMEUTILS_H
#define THEMEUTILS_H

#include <QObject>
#include <QPalette>
#include <QString>

namespace ThemeUtils {
QString qt6ctConfigDir();
QString qt6ctConfigPath();
QString readColorSchemePath(const QString &configPath);
QString resolveSchemePath(const QString &configDir, const QString &schemePath);
bool loadColorSchemePalette(QPalette &palette, const QString &configDir, QString *schemePathOut = nullptr);
bool applyQt6ctPalette(QString *schemePathOut = nullptr);
}

class QFileSystemWatcher;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    enum class ThemeMode {
        System,
        Light,
        Dark,
        SkyBlue,
        DeepBlue,
        ForestGreen,
        SunsetOrange,
        VioletPurple,
        NeutralGray,
        SakuraPink,
        LemonYellow
    };
    Q_ENUM(ThemeMode)

    explicit ThemeManager(QObject *parent = nullptr);

    void initialize();
    bool reload();
    void setThemeMode(ThemeMode mode);
    ThemeMode themeMode() const;
    ThemeMode effectiveThemeMode() const;
    QString themeModeKey() const;
    QString currentSchemePath() const;
    QString currentSchemeName() const;
    QString currentStyleName() const;

    static ThemeMode themeModeFromKey(const QString &key);
    static QString themeModeToKey(ThemeMode mode);
    static QString themeModeDisplayName(ThemeMode mode);

signals:
    void themeChanged();

private slots:
    void handleConfigChanged(const QString &path);

private:
    void updateWatchPaths();
    QString readConfigValue(const QString &key) const;

    QFileSystemWatcher *watcher_ = nullptr;
    QString configRoot_;
    QString configDir_;
    QString configPath_;
    QString schemePath_;
    QString styleName_;
    QPalette systemPalette_;
    bool systemPaletteCaptured_ = false;
    bool systemPaletteIsDark_ = false;
    ThemeMode themeMode_ = ThemeMode::System;
};

#endif // THEMEUTILS_H
