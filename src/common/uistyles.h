#ifndef UISTYLES_H
#define UISTYLES_H

#include <QString>

class QTabWidget;
class QWidget;

namespace UiStyles {
void applyDialogStyle(QWidget *widget);
void applyPageStyle(QWidget *widget);
void applyMainWindowStyle(QWidget *widget);
void applyTabStyle(QTabWidget *tabs);
QString titleTextStyle();
QString secondaryTextStyle();
QString metricTitleStyle();
QString metricValueStyle();
QString progressBarStyle(const QString &accentColor);
QString metricCardStyle(const QString &accentColor);
}

#endif // UISTYLES_H