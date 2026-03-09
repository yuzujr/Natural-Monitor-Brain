#include "databasemanager.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "common/uistyles.h"
#include "themeutils.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("JLU"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("jlu.edu.cn"));
    QCoreApplication::setApplicationName(QStringLiteral("EnvDataPlatform"));

    ThemeManager themeManager;
    themeManager.initialize();
    themeManager.reload();

    DatabaseManager db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("启动失败"), db.lastError());
        return 1;
    }

    LoginDialog login(&db);
    QObject::connect(&themeManager, &ThemeManager::themeChanged, &login, [&login]() {
        UiStyles::applyDialogStyle(&login);
        login.setPalette(QApplication::palette());
        login.update();
    });
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window(&db, &themeManager, login.currentUser());
    window.show();
    return app.exec();
}
