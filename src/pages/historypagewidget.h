#ifndef HISTORYPAGEWIDGET_H
#define HISTORYPAGEWIDGET_H

#include <QDateTime>
#include <QWidget>

#include "databasemanager.h"

class QCheckBox;
class QDateTimeEdit;
class QLabel;
class QTableWidget;

class HistoryPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPageWidget(QWidget *parent = nullptr);

    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    void setQueryResult(const QList<EnvSample> &samples, const EnvStats &stats, int limit);

signals:
    void queryRequested(const QDateTime &start, const QDateTime &end);

private:
    QDateTimeEdit *startEdit_ = nullptr;
    QDateTimeEdit *endEdit_ = nullptr;
    QCheckBox *tempCheck_ = nullptr;
    QCheckBox *humCheck_ = nullptr;
    QCheckBox *pmCheck_ = nullptr;
    QCheckBox *co2Check_ = nullptr;
    QTableWidget *historyTable_ = nullptr;
    QTableWidget *statsTable_ = nullptr;
    QLabel *countLabel_ = nullptr;
};

#endif // HISTORYPAGEWIDGET_H