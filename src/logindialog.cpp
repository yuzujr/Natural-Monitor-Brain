#include "logindialog.h"

#include "common/uistyles.h"
#include "languageutils.h"
#include "repositories/userrepository.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

LoginDialog::LoginDialog(DatabaseManager *db, LanguageManager *languageManager, QWidget *parent)
    : QDialog(parent)
    , db_(db)
    , languageManager_(languageManager)
    , userRepository_(db ? db->userRepository() : nullptr)
{
    setWindowTitle(tr("用户登录"));
    setModal(true);
    resize(420, 260);
    UiStyles::applyDialogStyle(this);

    auto *layout = new QVBoxLayout(this);
    tabs_ = new QTabWidget(this);

    auto *loginPage = new QWidget(this);
    auto *loginForm = new QFormLayout(loginPage);
    loginUserEdit_ = new QLineEdit(loginPage);
    loginPassEdit_ = new QLineEdit(loginPage);
    loginUserEdit_->setPlaceholderText(tr("请输入用户名"));
    loginPassEdit_->setPlaceholderText(tr("请输入密码"));
    loginPassEdit_->setEchoMode(QLineEdit::Password);
    loginForm->addRow(tr("用户名"), loginUserEdit_);
    loginForm->addRow(tr("密码"), loginPassEdit_);

    auto *loginInfo = new QLabel(tr("默认管理员: admin / admin123"), loginPage);
    loginInfo->setStyleSheet(UiStyles::secondaryTextStyle());
    loginForm->addRow(loginInfo);

    auto *loginButtons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, loginPage);
    loginButtons->button(QDialogButtonBox::Ok)->setText(tr("登录"));
    loginButtons->button(QDialogButtonBox::Cancel)->setText(tr("退出"));
    UiStyles::applyButtonVariant(loginButtons->button(QDialogButtonBox::Ok), QStringLiteral("primary"));
    UiStyles::applyButtonVariant(loginButtons->button(QDialogButtonBox::Cancel), QStringLiteral("secondary"));
    connect(loginButtons, &QDialogButtonBox::accepted, this, &LoginDialog::handleLogin);
    connect(loginButtons, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
    loginForm->addRow(loginButtons);

    auto *registerPage = new QWidget(this);
    auto *registerForm = new QFormLayout(registerPage);
    regUserEdit_ = new QLineEdit(registerPage);
    regPassEdit_ = new QLineEdit(registerPage);
    regPassConfirmEdit_ = new QLineEdit(registerPage);
    regUserEdit_->setPlaceholderText(tr("新用户名"));
    regPassEdit_->setPlaceholderText(tr("设置密码"));
    regPassConfirmEdit_->setPlaceholderText(tr("再次输入密码"));
    regPassEdit_->setEchoMode(QLineEdit::Password);
    regPassConfirmEdit_->setEchoMode(QLineEdit::Password);
    registerForm->addRow(tr("用户名"), regUserEdit_);
    registerForm->addRow(tr("密码"), regPassEdit_);
    registerForm->addRow(tr("确认密码"), regPassConfirmEdit_);

    auto *registerButtons = new QDialogButtonBox(QDialogButtonBox::Ok, registerPage);
    registerButtons->button(QDialogButtonBox::Ok)->setText(tr("注册"));
    UiStyles::applyButtonVariant(registerButtons->button(QDialogButtonBox::Ok), QStringLiteral("primary"));
    connect(registerButtons, &QDialogButtonBox::accepted, this, &LoginDialog::handleRegister);
    registerForm->addRow(registerButtons);

    auto *resetPage = new QWidget(this);
    auto *resetForm = new QFormLayout(resetPage);
    resetUserEdit_ = new QLineEdit(resetPage);
    resetPassEdit_ = new QLineEdit(resetPage);
    resetPassConfirmEdit_ = new QLineEdit(resetPage);
    resetUserEdit_->setPlaceholderText(tr("需要重置的用户名"));
    resetPassEdit_->setPlaceholderText(tr("输入新密码"));
    resetPassConfirmEdit_->setPlaceholderText(tr("再次输入新密码"));
    resetPassEdit_->setEchoMode(QLineEdit::Password);
    resetPassConfirmEdit_->setEchoMode(QLineEdit::Password);
    resetForm->addRow(tr("用户名"), resetUserEdit_);
    resetForm->addRow(tr("新密码"), resetPassEdit_);
    resetForm->addRow(tr("确认密码"), resetPassConfirmEdit_);

    auto *resetButtons = new QDialogButtonBox(QDialogButtonBox::Ok, resetPage);
    resetButtons->button(QDialogButtonBox::Ok)->setText(tr("重置密码"));
    UiStyles::applyButtonVariant(resetButtons->button(QDialogButtonBox::Ok), QStringLiteral("danger"));
    connect(resetButtons, &QDialogButtonBox::accepted, this, &LoginDialog::handleReset);
    resetForm->addRow(resetButtons);

    tabs_->addTab(loginPage, tr("登录"));
    tabs_->addTab(registerPage, tr("注册"));
    tabs_->addTab(resetPage, tr("找回密码"));

    layout->addWidget(tabs_);

    languageButton_ = new QPushButton(languageManager_ ? languageManager_->nextLanguageButtonText() : tr("切换到 English"), this);
    UiStyles::applyButtonVariant(languageButton_, QStringLiteral("secondary"));
    connect(languageButton_, &QPushButton::clicked, this, [this]() {
        if (languageManager_) {
            languageManager_->toggleLanguage();
            done(LanguageSwitchResult);
        }
    });
    layout->addWidget(languageButton_, 0, Qt::AlignRight);
}

UserInfo LoginDialog::currentUser() const
{
    return currentUser_;
}

void LoginDialog::handleLogin()
{
    if (!userRepository_) {
        QMessageBox::critical(this, tr("错误"), tr("数据库未初始化"));
        return;
    }

    const QString username = loginUserEdit_->text().trimmed();
    const QString password = loginPassEdit_->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请输入用户名和密码"));
        return;
    }

    UserInfo user;
    if (!userRepository_->validateUser(username, password, &user)) {
        QMessageBox::warning(this, tr("登录失败"), tr("用户名或密码错误"));
        return;
    }

    currentUser_ = user;
    accept();
}

void LoginDialog::handleRegister()
{
    if (!userRepository_) {
        QMessageBox::critical(this, tr("错误"), tr("数据库未初始化"));
        return;
    }

    const QString username = regUserEdit_->text().trimmed();
    const QString password = regPassEdit_->text();
    const QString confirm = regPassConfirmEdit_->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请输入用户名和密码"));
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, tr("提示"), tr("两次输入的密码不一致"));
        return;
    }

    if (!userRepository_->addUser(username, password, QStringLiteral("user"))) {
        QMessageBox::warning(this, tr("注册失败"), db_->lastError());
        return;
    }

    QMessageBox::information(this, tr("注册成功"), tr("账号已创建，请使用新账号登录"));
    regUserEdit_->clear();
    regPassEdit_->clear();
    regPassConfirmEdit_->clear();
    tabs_->setCurrentIndex(0);
}

void LoginDialog::handleReset()
{
    if (!userRepository_) {
        QMessageBox::critical(this, tr("错误"), tr("数据库未初始化"));
        return;
    }

    const QString username = resetUserEdit_->text().trimmed();
    const QString password = resetPassEdit_->text();
    const QString confirm = resetPassConfirmEdit_->text();
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请输入用户名和新密码"));
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, tr("提示"), tr("两次输入的密码不一致"));
        return;
    }

    if (!userRepository_->resetPassword(username, password)) {
        QMessageBox::warning(this, tr("重置失败"), tr("未找到该用户"));
        return;
    }

    QMessageBox::information(this, tr("重置成功"), tr("密码已更新，请重新登录"));
    resetUserEdit_->clear();
    resetPassEdit_->clear();
    resetPassConfirmEdit_->clear();
    tabs_->setCurrentIndex(0);
}

