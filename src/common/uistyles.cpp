#include "uistyles.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QPushButton>
#include <QStyle>
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

QColor primaryButtonColor()
{
    return isDarkTheme() ? QColor(QStringLiteral("#2c6da8")) : QColor(QStringLiteral("#a9d4fb"));
}

QColor primaryButtonHoverColor()
{
    return isDarkTheme() ? primaryButtonColor().lighter(110) : primaryButtonColor().darker(105);
}

QColor primaryButtonPressedColor()
{
    return isDarkTheme() ? primaryButtonColor().darker(114) : primaryButtonColor().darker(112);
}

QColor primaryButtonTextColor()
{
    return isDarkTheme() ? QColor(QStringLiteral("#f3f9ff")) : QColor(QStringLiteral("#102234"));
}

QColor secondaryButtonColor()
{
    return isDarkTheme() ? panelColor().lighter(108) : panelColor().darker(102);
}

QColor secondaryButtonHoverColor()
{
    return isDarkTheme() ? secondaryButtonColor().lighter(112) : secondaryButtonColor().darker(106);
}

QColor secondaryButtonPressedColor()
{
    return isDarkTheme() ? secondaryButtonColor().darker(112) : secondaryButtonColor().darker(112);
}

QColor dangerColor()
{
    return isDarkTheme() ? QColor(QStringLiteral("#cc6b74")) : QColor(QStringLiteral("#ba3b46"));
}

QColor dangerHoverColor()
{
    return isDarkTheme() ? dangerColor().lighter(110) : dangerColor().lighter(108);
}

QColor dangerPressedColor()
{
    return isDarkTheme() ? dangerColor().darker(116) : dangerColor().darker(112);
}

QColor dangerTextColor()
{
    return isDarkTheme() ? QColor(QStringLiteral("#fff4f6")) : QColor(QStringLiteral("#2f0d11"));
}

QColor subtleTextColor()
{
    const QPalette palette = QApplication::palette();
    QColor color = isDarkTheme() ? palette.color(QPalette::Text).lighter(125)
                                 : palette.color(QPalette::Mid).darker(150);
    if (isDarkTheme() && color.lightnessF() < 0.68) {
        color = color.lighter(125);
    }
    if (!isDarkTheme() && color.lightnessF() > 0.48) {
        color = color.darker(145);
    }
    return color;
}

QColor primaryTextColor()
{
    QColor color = QApplication::palette().color(QPalette::WindowText);
    if (isDarkTheme() && color.lightnessF() < 0.82) {
        color = color.lighter(125);
    }
    if (!isDarkTheme() && color.lightnessF() > 0.25) {
        color = color.darker(170);
    }
    return color;
}

QColor cardValueTextColor()
{
    QColor color = primaryTextColor();
    if (isDarkTheme()) {
        return color.lighter(108);
    }
    return color.darker(105);
}

QColor inputTextColor()
{
    QColor color = QApplication::palette().color(QPalette::Text);
    if (isDarkTheme() && color.lightnessF() < 0.8) {
        color = color.lighter(122);
    }
    if (!isDarkTheme() && color.lightnessF() > 0.28) {
        color = color.darker(165);
    }
    return color;
}

QString commonFontFamily()
{
    return QStringLiteral("\"Microsoft YaHei UI\", \"Segoe UI\", \"Noto Sans CJK SC\"");
}

QString buttonRules()
{
    return QString(
        "QPushButton {"
        "  background: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 8px 16px;"
        "  min-height: 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background: %3; }"
        "QPushButton:pressed { background: %4; }"
        "QPushButton[variant=\"secondary\"] { background: %5; color: %6; border: 1px solid %7; }"
        "QPushButton[variant=\"secondary\"]:hover { background: %8; }"
        "QPushButton[variant=\"secondary\"]:pressed { background: %9; }"
        "QPushButton[variant=\"danger\"] { background: %10; color: %13; }"
        "QPushButton[variant=\"danger\"]:hover { background: %11; }"
        "QPushButton[variant=\"danger\"]:pressed { background: %12; }"
    )
        .arg(colorToString(primaryButtonColor()))
        .arg(colorToString(primaryButtonTextColor()))
        .arg(colorToString(primaryButtonHoverColor()))
        .arg(colorToString(primaryButtonPressedColor()))
        .arg(colorToString(secondaryButtonColor()))
            .arg(colorToString(primaryTextColor()))
        .arg(colorToString(borderColor()))
        .arg(colorToString(secondaryButtonHoverColor()))
        .arg(colorToString(secondaryButtonPressedColor()))
        .arg(colorToString(dangerColor()))
        .arg(colorToString(dangerHoverColor()))
        .arg(colorToString(dangerPressedColor()))
        .arg(colorToString(dangerTextColor()));
}

QString calendarRules()
{
    return QString(
        "QDateTimeEdit::drop-down, QComboBox::drop-down {"
        "  border: none;"
        "  width: 22px;"
        "  background: transparent;"
        "}"
        "QDateTimeEdit::down-arrow, QComboBox::down-arrow {"
        "  image: none;"
        "  border-left: 5px solid transparent;"
        "  border-right: 5px solid transparent;"
        "  border-top: 6px solid %1;"
        "  width: 0px;"
        "  height: 0px;"
        "}"
        "QCalendarWidget QWidget {"
        "  background: %2;"
        "  color: %3;"
        "  alternate-background-color: %4;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "  background: %2;"
        "  color: %3;"
        "  selection-background-color: %5;"
        "  selection-color: %6;"
        "}"
        "QCalendarWidget QToolButton {"
        "  background: %4;"
        "  color: %3;"
        "  border: 1px solid %7;"
        "  border-radius: 8px;"
        "  padding: 4px 8px;"
        "}"
        "QCalendarWidget QToolButton:hover { background: %8; }"
        "QCalendarWidget QSpinBox {"
        "  background: %2;"
        "  color: %3;"
        "  border: 1px solid %7;"
        "  border-radius: 6px;"
        "  padding: 2px 6px;"
        "}"
        "QCalendarWidget QMenu { background: %2; color: %3; }"
        "QCalendarWidget QAbstractItemView:disabled { color: %9; }"
    )
        .arg(colorToString(primaryTextColor()))
        .arg(colorToString(panelColor()))
        .arg(colorToString(primaryTextColor()))
        .arg(colorToString(alternatePanelColor()))
        .arg(colorToString(accentColor()))
        .arg(isDarkTheme() ? QStringLiteral("#081018") : QStringLiteral("#ffffff"))
        .arg(colorToString(borderColor()))
        .arg(colorToString(secondaryButtonHoverColor()))
        .arg(colorToString(subtleTextColor()));
}

}

void applyDialogStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor base = QApplication::palette().color(QPalette::Base);
    const QColor text = primaryTextColor();
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
        "%11"
        "%12"
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
        .arg(buttonRules())
        .arg(calendarRules()));
}

void applyPageStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor text = primaryTextColor();
    const QColor base = QApplication::palette().color(QPalette::Base);
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
        "%6"
        "QLineEdit, QComboBox, QDateTimeEdit, QSpinBox, QDoubleSpinBox {"
            "  background: %8;"
        "  color: %7;"
        "  border: 1px solid %4;"
        "  border-radius: 10px;"
        "  padding: 6px 10px;"
        "  min-height: 18px;"
        "}"
        "QLineEdit[readOnly=\"true\"] { color: %5; }"
        "QTableWidget {"
            "  background: %8;"
        "  color: %1;"
        "  border: 1px solid %4;"
        "  border-radius: 14px;"
        "  gridline-color: %4;"
            "  alternate-background-color: %9;"
        "}"
        "QHeaderView::section {"
            "  background: %9;"
        "  color: %1;"
        "  border: none;"
        "  border-bottom: 1px solid %4;"
        "  padding: 8px;"
        "  font-weight: 600;"
        "}"
        "QCheckBox, QLabel { color: %1; }"
        "QAbstractItemView::item:selected { background: palette(highlight); color: palette(highlightedText); }"
        "%10"
    )
        .arg(colorToString(text))
        .arg(commonFontFamily())
        .arg(colorToString(panelColor()))
        .arg(colorToString(borderColor()))
        .arg(colorToString(subtleTextColor()))
        .arg(buttonRules())
        .arg(colorToString(inputTextColor()))
        .arg(colorToString(base))
        .arg(colorToString(alternatePanelColor()))
        .arg(calendarRules()));
}

void applyMainWindowStyle(QWidget *widget)
{
    if (!widget) {
        return;
    }

    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor text = primaryTextColor();

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
        .arg(colorToString(primaryTextColor()))
        .arg(colorToString(borderColor()))
        .arg(commonFontFamily())
        .arg(colorToString(panelColor()))
        .arg(colorToString(accentColor())));
}

void applyButtonVariant(QPushButton *button, const QString &variant)
{
    if (!button) {
        return;
    }

    button->setProperty("variant", variant);
    button->style()->unpolish(button);
    button->style()->polish(button);
    button->update();
}

QString titleTextStyle()
{
    return QString("font-size: 26px; font-weight: 700; color: %1;")
        .arg(colorToString(primaryTextColor()));
}

QString secondaryTextStyle()
{
    return QString("color: %1; font-size: 13px; padding: 2px 4px 6px 4px;")
        .arg(colorToString(subtleTextColor()));
}

QString metricTitleStyle()
{
    return QString("color: %1; font-size: 12px; font-weight: 600;")
        .arg(colorToString(subtleTextColor()));
}

QString metricValueStyle()
{
    return QString("font-size: 20px; font-weight: 700; color: %1;")
        .arg(colorToString(cardValueTextColor()));
}

QString progressBarStyle(const QString &accentColor)
{
    return QString(
        "QProgressBar { background: %2; border: none; border-radius: 6px; min-height: 10px; }"
        "QProgressBar::chunk { background: %1; border-radius: 6px; }"
    ).arg(accentColor, colorToString(alternatePanelColor()));
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
        .arg(colorToString(primaryTextColor()));
}

} // namespace UiStyles