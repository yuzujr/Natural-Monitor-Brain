#include "userrepository.h"

#include <QSqlError>
#include <QSqlQuery>

UserRepository::UserRepository(DatabaseManager *db)
    : db_(db)
{
}

bool UserRepository::validateUser(const QString &username, const QString &password, UserInfo *outUser)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("SELECT id, username, role, created_at FROM users "
                                 "WHERE username = :username AND password_hash = :hash"));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    query.bindValue(QStringLiteral(":hash"), db_->hashPassword(password));
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }

    if (!query.next()) {
        return false;
    }

    if (outUser) {
        outUser->id = query.value(0).toInt();
        outUser->username = query.value(1).toString();
        outUser->role = query.value(2).toString();
        outUser->createdAt = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong());
    }
    return true;
}

bool UserRepository::addUser(const QString &username, const QString &password, const QString &role)
{
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        db_->setLastError(QStringLiteral("用户名或密码不能为空"));
        return false;
    }

    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("INSERT INTO users (username, password_hash, role, created_at) "
                                 "VALUES (:username, :hash, :role, :created_at)"));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    query.bindValue(QStringLiteral(":hash"), db_->hashPassword(password));
    query.bindValue(QStringLiteral(":role"), role);
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTime().toMSecsSinceEpoch());
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return true;
}

bool UserRepository::resetPassword(const QString &username, const QString &newPassword)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("UPDATE users SET password_hash = :hash WHERE username = :username"));
    query.bindValue(QStringLiteral(":hash"), db_->hashPassword(newPassword));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<UserInfo> UserRepository::listUsers()
{
    QList<UserInfo> result;
    QSqlQuery query(db_->database());
    if (!query.exec(QStringLiteral("SELECT id, username, role, created_at FROM users ORDER BY role DESC, id ASC"))) {
        db_->setLastError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        UserInfo info;
        info.id = query.value(0).toInt();
        info.username = query.value(1).toString();
        info.role = query.value(2).toString();
        info.createdAt = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong());
        result.push_back(info);
    }
    return result;
}

bool UserRepository::updateUserRole(int id, const QString &role)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("UPDATE users SET role = :role WHERE id = :id"));
    query.bindValue(QStringLiteral(":role"), role);
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool UserRepository::deleteUser(int id)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("DELETE FROM users WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool UserRepository::setUserPassword(int id, const QString &newPassword)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("UPDATE users SET password_hash = :hash WHERE id = :id"));
    query.bindValue(QStringLiteral(":hash"), db_->hashPassword(newPassword));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}