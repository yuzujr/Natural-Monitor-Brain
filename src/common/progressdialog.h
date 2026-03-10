#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);

    void setLabelText(const QString &text);
    void setRange(int minimum, int maximum);
    void setValue(int value);
    void setProgress(int current, int total);
    void setIndeterminate(bool indeterminate);

signals:
    void cancelled();

private:
    QLabel *label_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
};

#endif // PROGRESSDIALOG_H
