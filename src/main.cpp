#include "databasemanager.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "common/uistyles.h"
#include "themeutils.h"

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include <functional>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName(QStringLiteral("JLU"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("jlu.edu.cn"));
    QCoreApplication::setApplicationName(QStringLiteral("EnvDataPlatform"));

    ThemeManager themeManager;
    themeManager.initialize();
    QSettings settings;
    const ThemeManager::ThemeMode startupThemeMode = ThemeManager::themeModeFromKey(
        settings.value(QStringLiteral("theme_mode"), QStringLiteral("system")).toString());
    if (startupThemeMode == themeManager.themeMode()) {
        themeManager.reload();
    } else {
        themeManager.setThemeMode(startupThemeMode);
    }

    DatabaseManager db;
    if (!db.open()) {
        QMessageBox::critical(nullptr, QObject::tr("启动失败"), db.lastError());
        return 1;
    }

    std::function<void()> launchLoginFlow;
    launchLoginFlow = [&app, &db, &themeManager, &launchLoginFlow]() {
        auto *login = new LoginDialog(&db);
        QObject::connect(&themeManager, &ThemeManager::themeChanged, login, [login]() {
            UiStyles::applyDialogStyle(login);
            login->setPalette(QApplication::palette());
            login->update();
        });

        if (login->exec() != QDialog::Accepted) {
            login->deleteLater();
            app.quit();
            return;
        }

        const UserInfo user = login->currentUser();
        login->deleteLater();

        auto *window = new MainWindow(&db, &themeManager, user);
        auto *relaunchAfterClose = new bool(false);
        QObject::connect(window, &MainWindow::logoutRequested, window, [window, relaunchAfterClose, &launchLoginFlow]() {
            *relaunchAfterClose = true;
            window->deleteLater();
            QTimer::singleShot(0, [launchLoginFlow]() mutable { launchLoginFlow(); });
        });
        QObject::connect(window, &QObject::destroyed, &app, [&app, relaunchAfterClose]() {
            const bool shouldQuit = !(*relaunchAfterClose);
            delete relaunchAfterClose;
            if (shouldQuit) {
                app.quit();
            }
        });
        window->show();
    };

    QTimer::singleShot(0, [&launchLoginFlow]() { launchLoginFlow(); });
    return app.exec();
}
