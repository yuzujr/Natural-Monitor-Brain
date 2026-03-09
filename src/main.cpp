#include "databasemanager.h"
#include "logindialog.h"
#include "mainwindow.h"
#include "common/uistyles.h"
#include "languageutils.h"
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
    LanguageManager languageManager;
    languageManager.initialize();
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
    std::function<void(const UserInfo &)> launchMainWindow;
    launchMainWindow = [&app, &db, &themeManager, &languageManager, &launchLoginFlow, &launchMainWindow](const UserInfo &user) {
        auto *window = new MainWindow(&db, &themeManager, &languageManager, user);
        auto *relaunchAfterClose = new bool(false);
        QObject::connect(window, &MainWindow::logoutRequested, window, [window, relaunchAfterClose, &launchLoginFlow]() {
            *relaunchAfterClose = true;
            window->deleteLater();
            QTimer::singleShot(0, [&launchLoginFlow]() { launchLoginFlow(); });
        });
        QObject::connect(window, &MainWindow::relaunchRequested, window, [window, relaunchAfterClose, user, &launchMainWindow]() {
            *relaunchAfterClose = true;
            window->deleteLater();
            QTimer::singleShot(0, [launchMainWindow, user]() mutable { launchMainWindow(user); });
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

    launchLoginFlow = [&app, &db, &themeManager, &languageManager, &launchLoginFlow, &launchMainWindow]() {
        auto *login = new LoginDialog(&db, &languageManager);
        QObject::connect(&themeManager, &ThemeManager::themeChanged, login, [login]() {
            UiStyles::applyDialogStyle(login);
            login->setPalette(QApplication::palette());
            login->update();
        });

        const int result = login->exec();
        if (result == LoginDialog::LanguageSwitchResult) {
            login->deleteLater();
            QTimer::singleShot(0, [&launchLoginFlow]() { launchLoginFlow(); });
            return;
        }

        if (result != QDialog::Accepted) {
            login->deleteLater();
            app.quit();
            return;
        }

        const UserInfo user = login->currentUser();
        login->deleteLater();

        launchMainWindow(user);
    };

    QTimer::singleShot(0, [&launchLoginFlow]() { launchLoginFlow(); });
    return app.exec();
}
