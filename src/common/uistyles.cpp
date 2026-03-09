#include "uistyles.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QTabWidget>
#include <QWidget>

namespace UiStyles {

namespace {

bool isDarkTheme()
{
    return QApplication::palette().color(QPalette::Window).lightnessF() < 0.5;
}

QString colorToString(const QColor &color)
{
    return color.name(QColor::HexRgb);
}

QColor panelColor()
{
    const QPalette palette = QApplication::palette();
    return isDarkTheme() ? palette.color(QPalette::Base).lighter(112)
                         : palette.color(QPalette::Base).lighter(102);
}

QColor alternatePanelColor()
{
    const QPalette palette = QApplication::palette();
    return isDarkTheme() ? palette.color(QPalette::Base).lighter(122)
                         : palette.color(QPalette::AlternateBase).darker(102);
}

QColor borderColor()
{
    const QPalette palette = QApplication::palette();
    return isDarkTheme() ? palette.color(QPalette::Mid).lighter(130)
                         : palette.color(QPalette::Midlight).darker(112);
}

QColor accentColor()
{
    const QPalette palette = QApplication::palette();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QColor accent = palette.color(QPalette::Accent);
    if (accent.isValid()) {
        return accent;
    }
#endif
    return isDarkTheme() ? QColor(QStringLiteral("#69b3ff")) : QColor(QStringLiteral("#1f6aa5"));
}

QColor accentHoverColor()
{
    return isDarkTheme() ? accentColor().lighter(112) : accentColor().lighter(118);
}

QColor accentPressedColor()
{
    return isDarkTheme() ? accentColor().darker(118) : accentColor().darker(115);
}

QColor subtleTextColor()
{
    const QPalette palette = QApplication::palette();
    return isDarkTheme() ? palette.color(QPalette::Text).lighter(135)
                         : palette.color(QPalette::Mid).darker(130);
}

QString commonFontFamily()
{
    return QStringLiteral("\"Microsoft YaHei UI\", \"Segoe UI\", \"Noto Sans CJK SC\"");
}

}

void applyDialogStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor base = QApplication::palette().color(QPalette::Base);
    const QColor text = QApplication::palette().color(QPalette::WindowText);
    const QColor tabIdle = alternatePanelColor();
    const QColor tabActive = panelColor();
    const QColor border = borderColor();

    widget->setStyleSheet(QString(
        "QDialog {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:1 %2);"
        "  color: %3;"
        "  font-family: %4;"
        "}"
        "QLabel { color: %3; }"
        "QTabWidget::pane {"
        "  border: 1px solid %5;"
        "  border-radius: 16px;"
        "  background: %6;"
        "}"
        "QTabBar::tab {"
        "  background: %7;"
        "  color: %3;"
        "  padding: 10px 16px;"
        "  border-top-left-radius: 12px;"
        "  border-top-right-radius: 12px;"
        "  margin-right: 4px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: %8;"
        "  color: %9;"
        "  font-weight: 700;"
        "}"
        "QLineEdit {"
        "  background: %10;"
        "  color: %3;"
        "  border: 1px solid %5;"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "}"
        "QPushButton {"
        "  background: %9;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 8px 16px;"
        "}"
        "QPushButton:hover { background: %11; }"
        "QPushButton:pressed { background: %12; }"
    )
        .arg(colorToString(window.lighter(isDarkTheme() ? 112 : 102)))
        .arg(colorToString(base))
        .arg(colorToString(text))
        .arg(commonFontFamily())
        .arg(colorToString(border))
        .arg(colorToString(panelColor()))
        .arg(colorToString(tabIdle))
        .arg(colorToString(tabActive))
        .arg(colorToString(accentColor()))
        .arg(colorToString(base))
        .arg(colorToString(accentHoverColor()))
        .arg(colorToString(accentPressedColor())));
}

void applyPageStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor text = QApplication::palette().color(QPalette::WindowText);
    const QColor base = QApplication::palette().color(QPalette::Base);
    const QColor buttonText = Qt::white;

    widget->setStyleSheet(QString(
        "QWidget { background: transparent; color: %1; font-family: %2; }"
        "QGroupBox {"
        "  background: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 16px;"
        "  margin-top: 14px;"
        "  padding: 18px 16px 16px 16px;"
        "  font-weight: 600;"
        "  color: %1;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 14px;"
        "  padding: 0 8px;"
        "  color: %5;"
        "}"
        "QPushButton {"
        "  background: %6;"
        "  color: %7;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 8px 16px;"
        "  min-height: 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: %8; }"
        "QPushButton:pressed { background: %9; }"
        "QLineEdit, QComboBox, QDateTimeEdit, QSpinBox, QDoubleSpinBox {"
        "  background: %10;"
        "  color: %1;"
        "  border: 1px solid %4;"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "  min-height: 18px;"
        "}"
        "QTableWidget {"
        "  background: %10;"
        "  color: %1;"
        "  border: 1px solid %4;"
        "  border-radius: 14px;"
        "  gridline-color: %4;"
        "  alternate-background-color: %11;"
        "}"
        "QHeaderView::section {"
        "  background: %11;"
        "  color: %1;"
        "  border: none;"
        "  border-bottom: 1px solid %4;"
        "  padding: 8px;"
        "  font-weight: 600;"
        "}"
        "QCheckBox, QLabel { color: %1; }"
    )
        .arg(colorToString(text))
        .arg(commonFontFamily())
        .arg(colorToString(panelColor()))
        .arg(colorToString(borderColor()))
        .arg(colorToString(subtleTextColor()))
        .arg(colorToString(accentColor()))
        .arg(colorToString(buttonText))
        .arg(colorToString(accentHoverColor()))
        .arg(colorToString(accentPressedColor()))
        .arg(colorToString(base))
        .arg(colorToString(alternatePanelColor())));
}

void applyMainWindowStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor text = QApplication::palette().color(QPalette::WindowText);

    widget->setStyleSheet(QString(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 %1, stop:0.55 %2, stop:1 %3);"
        "}"
        "QStatusBar {"
        "  background: %4;"
        "  border-top: 1px solid %5;"
        "  color: %6;"
        "  font-family: %7;"
        "}"
    )
        .arg(colorToString(window.lighter(isDarkTheme() ? 118 : 104)))
        .arg(colorToString(window))
        .arg(colorToString(window.darker(isDarkTheme() ? 112 : 104)))
        .arg(colorToString(panelColor()))
        .arg(colorToString(borderColor()))
        .arg(colorToString(text))
        .arg(commonFontFamily()));
}

void applyTabStyle(QTabWidget *tabs)
{
    if (!tabs) {
        return;
    }

    tabs->setDocumentMode(true);
    tabs->setStyleSheet(QString(
        "QTabWidget::pane { border: none; }"
        "QTabBar::tab {"
        "  background: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-bottom: none;"
        "  min-width: 104px;"
        "  padding: 10px 18px;"
        "  border-top-left-radius: 12px;"
        "  border-top-right-radius: 12px;"
        "  margin-right: 6px;"
        "  font-family: %4;"
        "}"
        "QTabBar::tab:selected {"
        "  background: %5;"
        "  color: %6;"
        "  font-weight: 700;"
        "}"
    )
        .arg(colorToString(alternatePanelColor()))
        .arg(colorToString(QApplication::palette().color(QPalette::WindowText)))
        .arg(colorToString(borderColor()))
        .arg(commonFontFamily())
        .arg(colorToString(panelColor()))
        .arg(colorToString(accentColor())));
}

QString titleTextStyle()
{
    return QStringLiteral("font-size: 26px; font-weight: 700; color: palette(windowText);");
}

QString secondaryTextStyle()
{
    return QStringLiteral("color: palette(mid); font-size: 13px; padding: 2px 4px 6px 4px;");
}

QString metricTitleStyle()
{
    return QStringLiteral("color: palette(mid); font-size: 12px; font-weight: 600;");
}

QString metricValueStyle()
{
    return QStringLiteral("font-size: 20px; font-weight: 700; color: palette(windowText);");
}

QString progressBarStyle(const QString &accentColor)
{
    return QStringLiteral(
        "QProgressBar { background: palette(alternate-base); border: none; border-radius: 6px; min-height: 10px; }"
        "QProgressBar::chunk { background: %1; border-radius: 6px; }"
    ).arg(accentColor);
}

QString metricCardStyle(const QString &accentColor)
{
    return QString(
               "QFrame {"
               "  background: %1;"
               "  border: 1px solid %2;"
               "  border-left: 6px solid %3;"
               "  border-radius: 18px;"
               "}"
               "QLabel { background: transparent; color: %4; }"
           )
        .arg(colorToString(panelColor()))
        .arg(colorToString(borderColor()))
        .arg(accentColor)
        .arg(colorToString(QApplication::palette().color(QPalette::WindowText)));
}

} // namespace UiStyles