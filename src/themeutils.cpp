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

QPalette createSkyBluePalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#e8f4fc"));
    const QColor windowText(QStringLiteral("#1a3a4a"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#d6ebf5"));
    const QColor button(QStringLiteral("#c5e3f2"));
    const QColor buttonText(QStringLiteral("#1a3a4a"));
    const QColor mid(QStringLiteral("#7eb8d0"));
    const QColor dark(QStringLiteral("#3d7a94"));
    const QColor highlight(QStringLiteral("#2196f3"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#1976d2"));
    const QColor placeholder(QStringLiteral("#5a8fa8"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createDeepBluePalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#0d1b2a"));
    const QColor windowText(QStringLiteral("#e0f2fe"));
    const QColor base(QStringLiteral("#142536"));
    const QColor alternateBase(QStringLiteral("#1c3045"));
    const QColor button(QStringLiteral("#243b52"));
    const QColor buttonText(QStringLiteral("#e0f2fe"));
    const QColor mid(QStringLiteral("#5c7a99"));
    const QColor dark(QStringLiteral("#080f18"));
    const QColor highlight(QStringLiteral("#38bdf8"));
    const QColor highlightedText(QStringLiteral("#0a1621"));
    const QColor link(QStringLiteral("#7dd3fc"));
    const QColor placeholder(QStringLiteral("#7896b5"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.lighter(112), mid, base.lighter(106),
                   alternateBase.lighter(108), button.lighter(110), mid, mid, dark,
                   highlight.darker(108), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createForestGreenPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#e8f5e9"));
    const QColor windowText(QStringLiteral("#1b3d1c"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#d4edda"));
    const QColor button(QStringLiteral("#c3e6cb"));
    const QColor buttonText(QStringLiteral("#1b3d1c"));
    const QColor mid(QStringLiteral("#7eb88a"));
    const QColor dark(QStringLiteral("#3d7a4a"));
    const QColor highlight(QStringLiteral("#4caf50"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#2e7d32"));
    const QColor placeholder(QStringLiteral("#5a8f65"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createSunsetOrangePalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#fff3e0"));
    const QColor windowText(QStringLiteral("#4a2c0a"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#ffe0b2"));
    const QColor button(QStringLiteral("#ffcc80"));
    const QColor buttonText(QStringLiteral("#4a2c0a"));
    const QColor mid(QStringLiteral("#d49a5c"));
    const QColor dark(QStringLiteral("#a06a32"));
    const QColor highlight(QStringLiteral("#ff8c00"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#e65100"));
    const QColor placeholder(QStringLiteral("#b07d4a"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createVioletPurplePalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#f3e5f5"));
    const QColor windowText(QStringLiteral("#3d1a4a"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#e1bee7"));
    const QColor button(QStringLiteral("#ce93d8"));
    const QColor buttonText(QStringLiteral("#3d1a4a"));
    const QColor mid(QStringLiteral("#a87db5"));
    const QColor dark(QStringLiteral("#7a4a8a"));
    const QColor highlight(QStringLiteral("#9c27b0"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#7b1fa2"));
    const QColor placeholder(QStringLiteral("#9a6aa8"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createNeutralGrayPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#f5f5f5"));
    const QColor windowText(QStringLiteral("#212121"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#e0e0e0"));
    const QColor button(QStringLiteral("#d6d6d6"));
    const QColor buttonText(QStringLiteral("#212121"));
    const QColor mid(QStringLiteral("#9e9e9e"));
    const QColor dark(QStringLiteral("#616161"));
    const QColor highlight(QStringLiteral("#607d8b"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#455a64"));
    const QColor placeholder(QStringLiteral("#757575"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createSakuraPinkPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#fce4ec"));
    const QColor windowText(QStringLiteral("#4a1a2e"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#f8bbd9"));
    const QColor button(QStringLiteral("#f48fb1"));
    const QColor buttonText(QStringLiteral("#4a1a2e"));
    const QColor mid(QStringLiteral("#c97b9a"));
    const QColor dark(QStringLiteral("#9a4a6a"));
    const QColor highlight(QStringLiteral("#e91e63"));
    const QColor highlightedText(QStringLiteral("#ffffff"));
    const QColor link(QStringLiteral("#c2185b"));
    const QColor placeholder(QStringLiteral("#b06a85"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
    return palette;
}

QPalette createLemonYellowPalette()
{
    QPalette palette;
    const QColor window(QStringLiteral("#fffde7"));
    const QColor windowText(QStringLiteral("#4a4a0a"));
    const QColor base(QStringLiteral("#ffffff"));
    const QColor alternateBase(QStringLiteral("#fff9c4"));
    const QColor button(QStringLiteral("#fff59d"));
    const QColor buttonText(QStringLiteral("#4a4a0a"));
    const QColor mid(QStringLiteral("#d4c96a"));
    const QColor dark(QStringLiteral("#a69a3a"));
    const QColor highlight(QStringLiteral("#ffc107"));
    const QColor highlightedText(QStringLiteral("#3d3d00"));
    const QColor link(QStringLiteral("#ff8f00"));
    const QColor placeholder(QStringLiteral("#b0a85a"));

    setGroupColors(palette, QPalette::Active, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Inactive, window, windowText, base, alternateBase,
                   button, buttonText, mid, dark, highlight, highlightedText, link, placeholder);
    setGroupColors(palette, QPalette::Disabled, window.darker(103), mid, base.darker(102),
                   alternateBase.darker(103), button.darker(102), mid, mid, dark,
                   highlight.darker(105), highlightedText, link.darker(105), placeholder);
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
        
        QPalette newPalette;
        if (useSystemPalette) {
            newPalette = systemPalette_;
        } else {
            switch (themeMode_) {
            case ThemeMode::Dark:
                newPalette = createDarkPalette();
                break;
            case ThemeMode::SkyBlue:
                newPalette = createSkyBluePalette();
                break;
            case ThemeMode::DeepBlue:
                newPalette = createDeepBluePalette();
                break;
            case ThemeMode::ForestGreen:
                newPalette = createForestGreenPalette();
                break;
            case ThemeMode::SunsetOrange:
                newPalette = createSunsetOrangePalette();
                break;
            case ThemeMode::VioletPurple:
                newPalette = createVioletPurplePalette();
                break;
            case ThemeMode::NeutralGray:
                newPalette = createNeutralGrayPalette();
                break;
            case ThemeMode::SakuraPink:
                newPalette = createSakuraPinkPalette();
                break;
            case ThemeMode::LemonYellow:
                newPalette = createLemonYellowPalette();
                break;
            default:
                newPalette = createLightPalette();
                break;
            }
        }
        QApplication::setPalette(newPalette);
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
    return themeModeDisplayName(themeMode_);
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
    if (normalized == QStringLiteral("skyblue")) {
        return ThemeMode::SkyBlue;
    }
    if (normalized == QStringLiteral("deepblue")) {
        return ThemeMode::DeepBlue;
    }
    if (normalized == QStringLiteral("forestgreen")) {
        return ThemeMode::ForestGreen;
    }
    if (normalized == QStringLiteral("sunsetorange")) {
        return ThemeMode::SunsetOrange;
    }
    if (normalized == QStringLiteral("violetpurple")) {
        return ThemeMode::VioletPurple;
    }
    if (normalized == QStringLiteral("neutralgray")) {
        return ThemeMode::NeutralGray;
    }
    if (normalized == QStringLiteral("sakurapink")) {
        return ThemeMode::SakuraPink;
    }
    if (normalized == QStringLiteral("lemonyellow")) {
        return ThemeMode::LemonYellow;
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
    case ThemeMode::SkyBlue:
        return QStringLiteral("skyblue");
    case ThemeMode::DeepBlue:
        return QStringLiteral("deepblue");
    case ThemeMode::ForestGreen:
        return QStringLiteral("forestgreen");
    case ThemeMode::SunsetOrange:
        return QStringLiteral("sunsetorange");
    case ThemeMode::VioletPurple:
        return QStringLiteral("violetpurple");
    case ThemeMode::NeutralGray:
        return QStringLiteral("neutralgray");
    case ThemeMode::SakuraPink:
        return QStringLiteral("sakurapink");
    case ThemeMode::LemonYellow:
        return QStringLiteral("lemonyellow");
    case ThemeMode::System:
    default:
        return QStringLiteral("system");
    }
}

QString ThemeManager::themeModeDisplayName(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::Light:
        return QObject::tr("默认亮色");
    case ThemeMode::Dark:
        return QObject::tr("默认暗色");
    case ThemeMode::SkyBlue:
        return QObject::tr("晴空蓝");
    case ThemeMode::DeepBlue:
        return QObject::tr("深海蓝");
    case ThemeMode::ForestGreen:
        return QObject::tr("森林绿");
    case ThemeMode::SunsetOrange:
        return QObject::tr("落日橙");
    case ThemeMode::VioletPurple:
        return QObject::tr("紫堇紫");
    case ThemeMode::NeutralGray:
        return QObject::tr("中性灰");
    case ThemeMode::SakuraPink:
        return QObject::tr("樱花粉");
    case ThemeMode::LemonYellow:
        return QObject::tr("柠檬黄");
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
