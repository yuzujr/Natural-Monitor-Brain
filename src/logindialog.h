#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include "databasemanager.h"

class QLineEdit;
class QPushButton;
class QTabWidget;
class LanguageManager;
class UserRepository;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    enum DialogResult {
        LanguageSwitchResult = 1001
    };

    explicit LoginDialog(DatabaseManager *db, LanguageManager *languageManager, QWidget *parent = nullptr);

    UserInfo currentUser() const;

private slots:
    void handleLogin();
    void handleRegister();
    void handleReset();

private:
    DatabaseManager *db_ = nullptr;
    LanguageManager *languageManager_ = nullptr;
    UserRepository *userRepository_ = nullptr;
    UserInfo currentUser_;

    QTabWidget *tabs_ = nullptr;

    QLineEdit *loginUserEdit_ = nullptr;
    QLineEdit *loginPassEdit_ = nullptr;

    QLineEdit *regUserEdit_ = nullptr;
    QLineEdit *regPassEdit_ = nullptr;
    QLineEdit *regPassConfirmEdit_ = nullptr;

    QLineEdit *resetUserEdit_ = nullptr;
    QLineEdit *resetPassEdit_ = nullptr;
    QLineEdit *resetPassConfirmEdit_ = nullptr;
    QPushButton *languageButton_ = nullptr;
};

#endif // LOGINDIALOG_H
