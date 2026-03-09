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
    explicit ThemeManager(QObject *parent = nullptr);

    void initialize();
    bool reload();
    QString currentSchemePath() const;
    QString currentSchemeName() const;
    QString currentStyleName() const;

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
};

#endif // THEMEUTILS_H
