#include "usermanagementpagewidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "common/uistyles.h"

UserManagementPageWidget::UserManagementPageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    auto *introLabel = new QLabel(tr("管理员可在此维护用户账号、角色与密码，普通用户不会看到该页面。"), this);
    introLabel->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(introLabel);

    auto *addGroup = new QGroupBox(tr("添加用户"), this);
    auto *addLayout = new QGridLayout(addGroup);

    newUserName_ = new QLineEdit(addGroup);
    newUserPass_ = new QLineEdit(addGroup);
    newUserPass_->setEchoMode(QLineEdit::Password);
    newUserRole_ = new QComboBox(addGroup);
    newUserRole_->addItem(tr("普通用户"), QStringLiteral("user"));
    newUserRole_->addItem(tr("管理员"), QStringLiteral("admin"));
    auto *addButton = new QPushButton(tr("添加"), addGroup);
    UiStyles::applyButtonVariant(addButton, QStringLiteral("primary"));
    connect(addButton, &QPushButton::clicked, this, [this]() {
        emit addUserRequested(newUserName_->text().trimmed(), newUserPass_->text(), newUserRole_->currentData().toString());
    });

    newUserName_->setPlaceholderText(tr("输入用户名"));
    newUserPass_->setPlaceholderText(tr("输入初始密码"));

    addLayout->addWidget(new QLabel(tr("用户名"), addGroup), 0, 0);
    addLayout->addWidget(newUserName_, 0, 1);
    addLayout->addWidget(new QLabel(tr("密码"), addGroup), 0, 2);
    addLayout->addWidget(newUserPass_, 0, 3);
    addLayout->addWidget(new QLabel(tr("角色"), addGroup), 0, 4);
    addLayout->addWidget(newUserRole_, 0, 5);
    addLayout->addWidget(addButton, 0, 6);

    layout->addWidget(addGroup);

    userTable_ = new QTableWidget(this);
    userTable_->setColumnCount(4);
    userTable_->setHorizontalHeaderLabels({tr("ID"), tr("用户名"), tr("角色"), tr("创建时间")});
    userTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userTable_->setAlternatingRowColors(true);
    layout->addWidget(userTable_);

    auto *actionsLayout = new QHBoxLayout();
    auto *deleteButton = new QPushButton(tr("删除用户"), this);
    auto *toggleRoleButton = new QPushButton(tr("切换权限"), this);
    auto *resetPasswordButton = new QPushButton(tr("重置密码"), this);
    UiStyles::applyButtonVariant(deleteButton, QStringLiteral("danger"));
    UiStyles::applyButtonVariant(toggleRoleButton, QStringLiteral("secondary"));
    UiStyles::applyButtonVariant(resetPasswordButton, QStringLiteral("secondary"));

    connect(deleteButton, &QPushButton::clicked, this, [this]() {
        const int userId = selectedUserId();
        if (userId < 0) {
            QMessageBox::information(this, tr("提示"), tr("请先选择一个用户"));
            return;
        }
        emit deleteUserRequested(userId);
    });

    connect(toggleRoleButton, &QPushButton::clicked, this, [this]() {
        const int userId = selectedUserId();
        const QString role = selectedUserRole();
        if (userId < 0 || role.isEmpty()) {
            QMessageBox::information(this, tr("提示"), tr("请先选择一个用户"));
            return;
        }
        emit toggleRoleRequested(userId, role);
    });

    connect(resetPasswordButton, &QPushButton::clicked, this, [this]() {
        const int userId = selectedUserId();
        if (userId < 0) {
            QMessageBox::information(this, tr("提示"), tr("请先选择一个用户"));
            return;
        }

        bool ok = false;
        const QString password = QInputDialog::getText(this, tr("重置密码"), tr("输入新密码"),
                                                       QLineEdit::Password, QString(), &ok);
        if (!ok || password.isEmpty()) {
            return;
        }

        emit resetPasswordRequested(userId, password);
    });

    actionsLayout->addStretch();
    actionsLayout->addWidget(deleteButton);
    actionsLayout->addWidget(toggleRoleButton);
    actionsLayout->addWidget(resetPasswordButton);
    layout->addLayout(actionsLayout);
}

void UserManagementPageWidget::setUsers(const QList<UserInfo> &users)
{
    userTable_->setRowCount(users.size());
    for (int row = 0; row < users.size(); ++row) {
        const UserInfo &user = users.at(row);
        userTable_->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
        userTable_->setItem(row, 1, new QTableWidgetItem(user.username));
        userTable_->setItem(row, 2, new QTableWidgetItem(user.role));
        userTable_->setItem(row, 3, new QTableWidgetItem(user.createdAt.toString("yyyy-MM-dd HH:mm:ss")));
    }

    newUserName_->clear();
    newUserPass_->clear();
}

int UserManagementPageWidget::selectedUserId() const
{
    const int row = userTable_->currentRow();
    if (row < 0 || !userTable_->item(row, 0)) {
        return -1;
    }
    return userTable_->item(row, 0)->text().toInt();
}

QString UserManagementPageWidget::selectedUserRole() const
{
    const int row = userTable_->currentRow();
    if (row < 0 || !userTable_->item(row, 2)) {
        return {};
    }
    return userTable_->item(row, 2)->text();
}