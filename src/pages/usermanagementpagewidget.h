#ifndef USERMANAGEMENTPAGEWIDGET_H
#define USERMANAGEMENTPAGEWIDGET_H

#include <QWidget>

#include "databasemanager.h"

class QComboBox;
class QLineEdit;
class QTableWidget;

class UserManagementPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPageWidget(QWidget *parent = nullptr);

    void setUsers(const QList<UserInfo> &users);

signals:
    void addUserRequested(const QString &username, const QString &password, const QString &role);
    void deleteUserRequested(int userId);
    void toggleRoleRequested(int userId, const QString &currentRole);
    void resetPasswordRequested(int userId, const QString &password);

private:
    int selectedUserId() const;
    QString selectedUserRole() const;

    QTableWidget *userTable_ = nullptr;
    QLineEdit *newUserName_ = nullptr;
    QLineEdit *newUserPass_ = nullptr;
    QComboBox *newUserRole_ = nullptr;
};

#endif // USERMANAGEMENTPAGEWIDGET_H