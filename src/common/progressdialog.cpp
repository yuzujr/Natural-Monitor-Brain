#include "progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("进度"));
    setMinimumWidth(400);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(12);

    label_ = new QLabel(this);
    label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(label_);

    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    layout->addWidget(progressBar_);

    cancelButton_ = new QPushButton(tr("取消"), this);
    connect(cancelButton_, &QPushButton::clicked, this, [this]() {
        emit cancelled();
        reject();
    });
    layout->addWidget(cancelButton_);
}

void ProgressDialog::setLabelText(const QString &text)
{
    label_->setText(text);
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    progressBar_->setRange(minimum, maximum);
}

void ProgressDialog::setValue(int value)
{
    progressBar_->setValue(value);
}

void ProgressDialog::setProgress(int current, int total)
{
    progressBar_->setRange(0, total);
    progressBar_->setValue(current);
}

void ProgressDialog::setIndeterminate(bool indeterminate)
{
    if (indeterminate) {
        progressBar_->setRange(0, 0);
    } else {
        progressBar_->setRange(0, 100);
    }
}
