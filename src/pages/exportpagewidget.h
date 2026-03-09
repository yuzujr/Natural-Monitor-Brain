#ifndef EXPORTPAGEWIDGET_H
#define EXPORTPAGEWIDGET_H

#include <QDateTime>
#include <QWidget>

#include "common/moduletypes.h"

class QCheckBox;
class QDateTimeEdit;

class ExportPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPageWidget(QWidget *parent = nullptr);

signals:
    void exportHistoryRequested(const QDateTime &start, const QDateTime &end, const SampleSelection &selection);
    void exportStatsRequested(const QDateTime &start, const QDateTime &end);
    void exportAlarmsRequested(const QDateTime &start, const QDateTime &end);

private:
    SampleSelection currentSelection() const;

    QDateTimeEdit *historyStartEdit_ = nullptr;
    QDateTimeEdit *historyEndEdit_ = nullptr;
    QCheckBox *tempCheck_ = nullptr;
    QCheckBox *humCheck_ = nullptr;
    QCheckBox *pmCheck_ = nullptr;
    QCheckBox *co2Check_ = nullptr;
    QDateTimeEdit *alarmStartEdit_ = nullptr;
    QDateTimeEdit *alarmEndEdit_ = nullptr;
};

#endif // EXPORTPAGEWIDGET_H