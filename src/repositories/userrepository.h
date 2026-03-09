#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QList>
#include <QString>

#include "databasemanager.h"

class DatabaseManager;

class UserRepository
{
public:
    explicit UserRepository(DatabaseManager *db);

    bool validateUser(const QString &username, const QString &password, UserInfo *outUser);
    bool addUser(const QString &username, const QString &password, const QString &role);
    bool resetPassword(const QString &username, const QString &newPassword);
    QList<UserInfo> listUsers();
    bool updateUserRole(int id, const QString &role);
    bool deleteUser(int id);
    bool setUserPassword(int id, const QString &newPassword);

private:
    DatabaseManager *db_ = nullptr;
};

#endif // USERREPOSITORY_H