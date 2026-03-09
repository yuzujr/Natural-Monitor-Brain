#include "themeutils.h"

#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyle>
#include <QTextStream>
#include <QtGlobal>

namespace ThemeUtils {

QString qt6ctConfigDir()
{
    const QString configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configRoot.isEmpty()) {
        return {};
    }
    return QDir(configRoot).filePath(QStringLiteral("qt6ct"));
}

QString qt6ctConfigPath()
{
    const QString dir = qt6ctConfigDir();
    if (dir.isEmpty()) {
        return {};
    }
    return QDir(dir).filePath(QStringLiteral("qt6ct.conf"));
}

QString readColorSchemePath(const QString &configPath)
{
    if (configPath.isEmpty() || !QFile::exists(configPath)) {
        return {};
    }

    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }
        if (line.startsWith(QStringLiteral("color_scheme_path="))) {
            return line.mid(QStringLiteral("color_scheme_path=").size()).trimmed();
        }
    }
    return {};
}

QString resolveSchemePath(const QString &configDir, const QString &schemePath)
{
    if (schemePath.isEmpty()) {
        return {};
    }
    if (QDir::isRelativePath(schemePath)) {
        return QDir(configDir).filePath(schemePath);
    }
    return schemePath;
}

bool loadColorSchemePalette(QPalette &palette, const QString &configDir, QString *schemePathOut)
{
    const QString configPath = QDir(configDir).filePath(QStringLiteral("qt6ct.conf"));
    const QString schemePathRaw = readColorSchemePath(configPath);
    const QString schemePath = resolveSchemePath(configDir, schemePathRaw);

    if (schemePathOut) {
        *schemePathOut = schemePath;
    }

    if (schemePath.isEmpty() || !QFile::exists(schemePath)) {
        return false;
    }

    QFile file(schemePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString activeLine;
    QString inactiveLine;
    QString disabledLine;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }
        if (line.startsWith(QStringLiteral("active_colors="))) {
            activeLine = line.mid(QStringLiteral("active_colors=").size()).trimmed();
        } else if (line.startsWith(QStringLiteral("inactive_colors="))) {
            inactiveLine = line.mid(QStringLiteral("inactive_colors=").size()).trimmed();
        } else if (line.startsWith(QStringLiteral("disabled_colors="))) {
            disabledLine = line.mid(QStringLiteral("disabled_colors=").size()).trimmed();
        }
    }

    QVector<QPalette::ColorRole> roles;
    roles << QPalette::WindowText
          << QPalette::Button
          << QPalette::Light
          << QPalette::Midlight
          << QPalette::Dark
          << QPalette::Mid
          << QPalette::Text
          << QPalette::BrightText
          << QPalette::ButtonText
          << QPalette::Base
          << QPalette::Window
          << QPalette::Shadow
          << QPalette::Highlight
          << QPalette::HighlightedText
          << QPalette::Link
          << QPalette::LinkVisited
          << QPalette::AlternateBase
          << QPalette::ToolTipBase
          << QPalette::ToolTipText
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
          << QPalette::PlaceholderText
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
          << QPalette::Accent
#endif
        ;

    auto parseColors = [](const QString &value) {
        QList<QColor> colors;
        const QStringList parts = value.split(',', Qt::SkipEmptyParts);
        for (const QString &part : parts) {
            const QColor color(part.trimmed());
            if (color.isValid()) {
                colors.append(color);
            }
        }
        return colors;
    };

    auto applyColors = [&](QPalette::ColorGroup group, const QString &value) {
        const QList<QColor> colors = parseColors(value);
        const int count = qMin(colors.size(), roles.size());
        for (int i = 0; i < count; ++i) {
            palette.setColor(group, roles.at(i), colors.at(i));
        }
    };

    bool applied = false;
    if (!activeLine.isEmpty()) {
        applyColors(QPalette::Active, activeLine);
        applied = true;
    }
    if (!inactiveLine.isEmpty()) {
        applyColors(QPalette::Inactive, inactiveLine);
        applied = true;
    }
    if (!disabledLine.isEmpty()) {
        applyColors(QPalette::Disabled, disabledLine);
        applied = true;
    }

    return applied;
}

bool applyQt6ctPalette(QString *schemePathOut)
{
    QPalette palette;
    const QString configDir = qt6ctConfigDir();
    if (configDir.isEmpty()) {
        QApplication::setPalette(QPalette());
        return false;
    }

    const bool loaded = loadColorSchemePalette(palette, configDir, schemePathOut);
    QApplication::setPalette(loaded ? palette : QPalette());
    return loaded;
}

} // namespace ThemeUtils

namespace {

void setGroupColors(QPalette &palette, QPalette::ColorGroup group,
                    const QColor &window, const QColor &windowText,
                    const QColor &base, const QColor &alternateBase,
                    const QColor &button, const QColor &buttonText,
                    const QColor &mid, const QColor &dark,
                    const QColor &highlight, const QColor &highlightedText,
                    const QColor &link, const QColor &placeholder)
{
    palette.setColor(group, QPalette::Window, window);
    palette.setColor(group, QPalette::WindowText, windowText);
    palette.setColor(group, QPalette::Base, base);
    palette.setColor(group, QPalette::AlternateBase, alternateBase);
    palette.setColor(group, QPalette::ToolTipBase, base);
    palette.setColor(group, QPalette::ToolTipText, windowText);
    palette.setColor(group, QPalette::Text, windowText);
    palette.setColor(group, QPalette::Button, button);
    palette.setColor(group, QPalette::ButtonText, buttonText);
    palette.setColor(group, QPalette::BrightText, Qt::white);
    palette.setColor(group, QPalette::Light, window.lighter(140));
    palette.setColor(group, QPalette::Midlight, window.lighter(118));
    palette.setColor(group, QPalette::Mid, mid);
    palette.setColor(group, QPalette::Dark, dark);
    palette.setColor(group, QPalette::Shadow, dark.darker(125));
    palette.setColor(group, QPalette::Highlight, highlight);
    palette.setColor(group, QPalette::HighlightedText, highlightedText);
    palette.setColor(group, QPalette::Link, link);
    palette.setColor(group, QPalette::LinkVisited, link.darker(115));
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    palette.setColor(group, QPalette::PlaceholderText, placeholder);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    palette.setColor(group, QPalette::Accent, highlight);
#endif
}

QPalette createLightPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#edf3f9"));
    const QColor windowText(QStringLiteral("#132033"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#e3ebf4"));
    const QColor button(QStringLiteral("#d9e4f0"));
    const QColor buttonText(QStringLiteral("#132033"));
    const QColor mid(QStringLiteral("#8aa0b7"));
    const QColor dark(QStringLiteral("#4b6179"));
    const QColor highlight(QStringLiteral("#1f6aa5"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#0f5f9d"));
    const QColor placeholder(QStringLiteral("#6d7f92"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createDarkPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#101722"));
    const QColor windowText(QStringLiteral("#eef5ff"));
    const QColor base(QStringLiteral("#152030"));
    const QColor alternateBase(QStringLiteral("#1b2a3d"));
    const QColor button(QStringLiteral("#223247"));
    const QColor buttonText(QStringLiteral("#eef5ff"));
    const QColor mid(QStringLiteral("#7187a1"));
    const QColor dark(QStringLiteral("#07111d"));
    const QColor highlight(QStringLiteral("#56adff"));
    const QColor highlightedText(QStringLiteral("#081018"));
    const QColor link(QStringLiteral("#7cc4ff"));
    const QColor placeholder(QStringLiteral("#8fa3b9"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.lighter(112), mid, base.lighter(106),
                   alternateBase.lighter(108), button.lighter(110), mid, mid, dark,
                   highlight.darker(108), highlightedText, link.darker(105), placeholder);
    return palette;
}

}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
}

void ThemeManager::initialize()
{
    if (watcher_) {
        return;
    }

    if (QStyle *fusionStyle = QStyleFactory::create(QStringLiteral("Fusion"))) {
        QApplication::setStyle(fusionStyle);
    }

    watcher_ = new QFileSystemWatcher(this);
    configRoot_ = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (!systemPaletteCaptured_) {
        systemPalette_ = QApplication::palette();
        systemPaletteCaptured_ = true;
        systemPaletteIsDark_ = systemPalette_.color(QPalette::Window).lightnessF() < 0.5;
    }
    styleName_ = QApplication::style()->objectName();
    if (configRoot_.isEmpty()) {
        return;
    }

    configDir_ = QDir(configRoot_).filePath(QStringLiteral("qt6ct"));
    configPath_ = QDir(configDir_).filePath(QStringLiteral("qt6ct.conf"));

    if (QFile::exists(configPath_)) {
        watcher_->addPath(configPath_);
    }
    if (QDir(configDir_).exists()) {
        watcher_->addPath(configDir_);
    } else {
        watcher_->addPath(configRoot_);
    }

    updateWatchPaths();

    connect(watcher_, &QFileSystemWatcher::fileChanged, this, &ThemeManager::handleConfigChanged);
    connect(watcher_, &QFileSystemWatcher::directoryChanged, this, &ThemeManager::handleConfigChanged);
}

bool ThemeManager::reload()
{
    if (!watcher_) {
        initialize();
    }

    updateWatchPaths();

    bool loaded = false;
    if (themeMode_ == ThemeMode::System) {
        if (systemPaletteCaptured_) {
            QApplication::setPalette(systemPalette_);
            loaded = true;
        } else {
            QApplication::setPalette(QPalette());
        }
    } else {
        const bool useSystemPalette = (themeMode_ == ThemeMode::Dark && systemPaletteIsDark_)
            || (themeMode_ == ThemeMode::Light && !systemPaletteIsDark_);
        QApplication::setPalette(useSystemPalette
                                     ? systemPalette_
                                     : (themeMode_ == ThemeMode::Dark ? createDarkPalette() : createLightPalette()));
        loaded = true;
    }

    schemePath_.clear();

    emit themeChanged();
    return loaded;
}

void ThemeManager::setThemeMode(ThemeMode mode)
{
    if (themeMode_ == mode) {
        return;
    }
    themeMode_ = mode;
    reload();
}

ThemeManager::ThemeMode ThemeManager::themeMode() const
{
    return themeMode_;
}

ThemeManager::ThemeMode ThemeManager::effectiveThemeMode() const
{
    if (themeMode_ == ThemeMode::System) {
        return systemPaletteIsDark_ ? ThemeMode::Dark : ThemeMode::Light;
    }
    return themeMode_;
}

QString ThemeManager::themeModeKey() const
{
    return themeModeToKey(themeMode_);
}

QString ThemeManager::currentSchemePath() const
{
    return schemePath_;
}

QString ThemeManager::currentSchemeName() const
{
    if (themeMode_ == ThemeMode::System) {
        return systemPaletteIsDark_ ? tr("系统默认深色") : tr("系统默认浅色");
    }
    if (themeMode_ == ThemeMode::Light) {
        return (!systemPaletteIsDark_ && systemPaletteCaptured_) ? tr("系统默认浅色") : tr("优化浅色");
    }
    if (themeMode_ == ThemeMode::Dark) {
        return (systemPaletteIsDark_ && systemPaletteCaptured_) ? tr("系统默认深色") : tr("优化深色");
    }
    return schemePath_.isEmpty() ? QString() : QFileInfo(schemePath_).fileName();
}

QString ThemeManager::currentStyleName() const
{
    return QApplication::style()->objectName();
}

ThemeManager::ThemeMode ThemeManager::themeModeFromKey(const QString &key)
{
    const QString normalized = key.trimmed().toLower();
    if (normalized == QStringLiteral("light")) {
        return ThemeMode::Light;
    }
    if (normalized == QStringLiteral("dark")) {
        return ThemeMode::Dark;
    }
    return ThemeMode::System;
}

QString ThemeManager::themeModeToKey(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::Light:
        return QStringLiteral("light");
    case ThemeMode::Dark:
        return QStringLiteral("dark");
    case ThemeMode::System:
    default:
        return QStringLiteral("system");
    }
}

QString ThemeManager::themeModeDisplayName(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::Light:
        return QObject::tr("浅色");
    case ThemeMode::Dark:
        return QObject::tr("深色");
    case ThemeMode::System:
    default:
        return QObject::tr("跟随系统");
    }
}

void ThemeManager::handleConfigChanged(const QString &path)
{
    Q_UNUSED(path);
    updateWatchPaths();
    reload();
}

void ThemeManager::updateWatchPaths()
{
    if (!watcher_) {
        return;
    }

    if (!configPath_.isEmpty() && QFile::exists(configPath_)
        && !watcher_->files().contains(configPath_)) {
        watcher_->addPath(configPath_);
    }

    if (!configDir_.isEmpty() && QDir(configDir_).exists()
        && !watcher_->directories().contains(configDir_)) {
        watcher_->addPath(configDir_);
    }

    const QString schemeRaw = ThemeUtils::readColorSchemePath(configPath_);
    const QString schemePath = ThemeUtils::resolveSchemePath(configDir_, schemeRaw);
    if (schemePath.isEmpty()) {
        return;
    }

    if (schemePath != schemePath_) {
        if (!schemePath_.isEmpty()) {
            watcher_->removePath(schemePath_);
        }
        schemePath_ = schemePath;
    }

    if (QFile::exists(schemePath_) && !watcher_->files().contains(schemePath_)) {
        watcher_->addPath(schemePath_);
    }
}

QString ThemeManager::readConfigValue(const QString &key) const
{
    if (configPath_.isEmpty() || !QFile::exists(configPath_)) {
        return {};
    }

    QFile file(configPath_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString prefix = key + '=';
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }
        if (line.startsWith(prefix)) {
            return line.mid(prefix.size()).trimmed();
        }
    }
    return {};
}
