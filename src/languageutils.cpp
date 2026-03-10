#include "languageutils.h"

#include <QApplication>
#include <QHash>
#include <QSettings>

namespace {
QHash<QString, QString> buildEnglishMap()
{
    return {
        {QStringLiteral("启动失败"), QStringLiteral("Startup Failed")},
        {QStringLiteral("用户登录"), QStringLiteral("User Login")},
        {QStringLiteral("请输入用户名"), QStringLiteral("Enter username")},
        {QStringLiteral("请输入密码"), QStringLiteral("Enter password")},
        {QStringLiteral("用户名"), QStringLiteral("Username")},
        {QStringLiteral("密码"), QStringLiteral("Password")},
        {QStringLiteral("默认管理员: admin / admin123"), QStringLiteral("Default admin: admin / admin123")},
        {QStringLiteral("登录"), QStringLiteral("Login")},
        {QStringLiteral("退出"), QStringLiteral("Exit")},
        {QStringLiteral("新用户名"), QStringLiteral("New username")},
        {QStringLiteral("设置密码"), QStringLiteral("Set password")},
        {QStringLiteral("再次输入密码"), QStringLiteral("Re-enter password")},
        {QStringLiteral("确认密码"), QStringLiteral("Confirm Password")},
        {QStringLiteral("注册"), QStringLiteral("Register")},
        {QStringLiteral("需要重置的用户名"), QStringLiteral("Username to reset")},
        {QStringLiteral("输入新密码"), QStringLiteral("Enter new password")},
        {QStringLiteral("新密码"), QStringLiteral("New Password")},
        {QStringLiteral("重置密码"), QStringLiteral("Reset Password")},
        {QStringLiteral("找回密码"), QStringLiteral("Recover Password")},
        {QStringLiteral("错误"), QStringLiteral("Error")},
        {QStringLiteral("数据库未初始化"), QStringLiteral("Database is not initialized")},
        {QStringLiteral("提示"), QStringLiteral("Notice")},
        {QStringLiteral("请输入用户名和密码"), QStringLiteral("Please enter username and password")},
        {QStringLiteral("登录失败"), QStringLiteral("Login Failed")},
        {QStringLiteral("用户名或密码错误"), QStringLiteral("Incorrect username or password")},
        {QStringLiteral("两次输入的密码不一致"), QStringLiteral("The two passwords do not match")},
        {QStringLiteral("注册失败"), QStringLiteral("Registration Failed")},
        {QStringLiteral("注册成功"), QStringLiteral("Registration Successful")},
        {QStringLiteral("账号已创建，请使用新账号登录"), QStringLiteral("Account created. Please log in with the new account")},
        {QStringLiteral("请输入用户名和新密码"), QStringLiteral("Please enter username and new password")},
        {QStringLiteral("重置失败"), QStringLiteral("Reset Failed")},
        {QStringLiteral("未找到该用户"), QStringLiteral("User not found")},
        {QStringLiteral("重置成功"), QStringLiteral("Reset Successful")},
        {QStringLiteral("密码已更新，请重新登录"), QStringLiteral("Password updated. Please log in again")},
        {QStringLiteral("切换到 English"), QStringLiteral("Switch to English")},
        {QStringLiteral("Switch to 中文"), QStringLiteral("Switch to Chinese")},
        {QStringLiteral("环境数据信息平台"), QStringLiteral("Environmental Data Platform")},
        {QStringLiteral("实时数据"), QStringLiteral("Realtime")},
        {QStringLiteral("历史查询"), QStringLiteral("History")},
        {QStringLiteral("异常报警"), QStringLiteral("Alarms")},
        {QStringLiteral("数据导出"), QStringLiteral("Export")},
        {QStringLiteral("系统设置"), QStringLiteral("Settings")},
        {QStringLiteral("用户管理"), QStringLiteral("Users")},
        {QStringLiteral("文件"), QStringLiteral("File")},
        {QStringLiteral("注销"), QStringLiteral("Logout")},
        {QStringLiteral("帮助"), QStringLiteral("Help")},
        {QStringLiteral("关于"), QStringLiteral("About")},
        {QStringLiteral("环境数据信息平台\n基于Qt Widgets与SQLite实现"), QStringLiteral("Environmental Data Platform\nBuilt with Qt Widgets and SQLite")},
        {QStringLiteral("已登录: %1 (%2)"), QStringLiteral("Logged in: %1 (%2)")},
        {QStringLiteral("数据来源: 本地 UDP 服务 (127.0.0.1:8888)"), QStringLiteral("Source: Local UDP Service (127.0.0.1:8888)")},
        {QStringLiteral("已接收本地 UDP 数据: %1"), QStringLiteral("Received local UDP data: %1")},
        {QStringLiteral("数据来源: 本地 UDP 服务 (127.0.0.1:8888) [连接异常]"), QStringLiteral("Source: Local UDP Service (127.0.0.1:8888) [Connection Error]")},
        {QStringLiteral("开始时间不能晚于结束时间"), QStringLiteral("Start time cannot be later than end time")},
        {QStringLiteral("确认"), QStringLiteral("Confirm")},
        {QStringLiteral("确定要清空报警记录吗？"), QStringLiteral("Clear all alarm records?")},
        {QStringLiteral("失败"), QStringLiteral("Failed")},
        {QStringLiteral("已保存"), QStringLiteral("Saved")},
        {QStringLiteral("报警阈值已保存"), QStringLiteral("Alarm thresholds have been saved")},
        {QStringLiteral("导出历史数据"), QStringLiteral("Export History Data")},
        {QStringLiteral("CSV 文件 (*.csv)"), QStringLiteral("CSV Files (*.csv)")},
        {QStringLiteral("导出失败"), QStringLiteral("Export Failed")},
        {QStringLiteral("无法写入CSV文件"), QStringLiteral("Unable to write CSV file")},
        {QStringLiteral("导出成功"), QStringLiteral("Export Successful")},
        {QStringLiteral("历史数据已导出"), QStringLiteral("History data exported")},
        {QStringLiteral("导出统计结果"), QStringLiteral("Export Statistics")},
        {QStringLiteral("统计结果已导出"), QStringLiteral("Statistics exported")},
        {QStringLiteral("导出报警记录"), QStringLiteral("Export Alarm Records")},
        {QStringLiteral("报警记录已导出"), QStringLiteral("Alarm records exported")},
        {QStringLiteral("设置已应用"), QStringLiteral("Settings applied")},
        {QStringLiteral("备份数据库"), QStringLiteral("Backup Database")},
        {QStringLiteral("数据库文件 (*.db)"), QStringLiteral("Database Files (*.db)")},
        {QStringLiteral("数据库备份失败"), QStringLiteral("Database backup failed")},
        {QStringLiteral("成功"), QStringLiteral("Success")},
        {QStringLiteral("数据库已备份"), QStringLiteral("Database backed up")},
        {QStringLiteral("恢复数据库"), QStringLiteral("Restore Database")},
        {QStringLiteral("恢复数据库会覆盖现有数据，是否继续？"), QStringLiteral("Restoring the database will overwrite existing data. Continue?")},
        {QStringLiteral("无法删除原数据库"), QStringLiteral("Unable to remove the original database")},
        {QStringLiteral("数据库恢复失败"), QStringLiteral("Database restore failed")},
        {QStringLiteral("数据库恢复后重新连接失败"), QStringLiteral("Failed to reconnect after database restore")},
        {QStringLiteral("数据库已恢复"), QStringLiteral("Database restored")},
        {QStringLiteral("确认注销"), QStringLiteral("Confirm Logout")},
        {QStringLiteral("确定要注销当前账号并返回登录界面吗？"), QStringLiteral("Log out the current account and return to login?")},
        {QStringLiteral("添加失败"), QStringLiteral("Add Failed")},
        {QStringLiteral("不能删除当前登录用户"), QStringLiteral("The current logged-in user cannot be deleted")},
        {QStringLiteral("确定删除该用户吗？"), QStringLiteral("Delete this user?")},
        {QStringLiteral("密码已更新"), QStringLiteral("Password updated")},
        {QStringLiteral("将删除%1天之前的历史数据，是否继续？"), QStringLiteral("This will delete history data older than %1 days. Continue?")},
        {QStringLiteral("完成"), QStringLiteral("Completed")},
        {QStringLiteral("历史数据已清理"), QStringLiteral("History data cleaned up")},
        {QStringLiteral("跟随系统默认主题"), QStringLiteral("Follow system theme")},
        {QStringLiteral("未检测到"), QStringLiteral("Not detected")},
        {QStringLiteral("%1，当前效果: %2，样式: %3"), QStringLiteral("%1, current effect: %2, style: %3")},
        {QStringLiteral("[%1] %2 超过阈值: 当前值 %3，阈值 %4"), QStringLiteral("[%1] %2 exceeded threshold: current %3, threshold %4")},
        {QStringLiteral("多指标预警报告"), QStringLiteral("Multi-metric Alert Report")},
        {QStringLiteral("预警报告"), QStringLiteral("Alert Report")},
        {QStringLiteral("本次检测发现 %1 项异常：\n%2"), QStringLiteral("%1 anomalies were detected this round:\n%2")},
        {QStringLiteral("环境态势总览"), QStringLiteral("Environmental Overview")},
        {QStringLiteral("环境态势趋势"), QStringLiteral("Environmental Trends")},
        {QStringLiteral("实时监控温度、湿度、PM2.5 与 CO2 变化，支持快速切换刷新节奏。"), QStringLiteral("Monitor temperature, humidity, PM2.5 and CO2 in real time with quick refresh switching.")},
        {QStringLiteral("暂停采集"), QStringLiteral("Pause Collection")},
        {QStringLiteral("继续采集"), QStringLiteral("Resume Collection")},
        {QStringLiteral("1 秒"), QStringLiteral("1 s")},
        {QStringLiteral("5 秒"), QStringLiteral("5 s")},
        {QStringLiteral("10 秒"), QStringLiteral("10 s")},
        {QStringLiteral("刷新频率"), QStringLiteral("Refresh Rate")},
        {QStringLiteral("当前环境数据"), QStringLiteral("Current Environment Data")},
        {QStringLiteral("温度 (C)"), QStringLiteral("Temperature (C)")},
        {QStringLiteral("湿度 (%)"), QStringLiteral("Humidity (%)")},
        {QStringLiteral("PM2.5 (ug/m3)"), QStringLiteral("PM2.5 (ug/m3)")},
        {QStringLiteral("CO2 (ppm)"), QStringLiteral("CO2 (ppm)")},
        {QStringLiteral("趋势概览"), QStringLiteral("Trend Overview")},
        {QStringLiteral("最近更新时间: --"), QStringLiteral("Last Update: --")},
        {QStringLiteral("最近更新时间: %1"), QStringLiteral("Last Update: %1")},
        {QStringLiteral("按时间范围检索历史数据，并查看同一时间段内各项指标的统计结果。"), QStringLiteral("Query historical data by time range and view statistics for the same period.")},
        {QStringLiteral("查询条件"), QStringLiteral("Query Filters")},
        {QStringLiteral("开始时间"), QStringLiteral("Start Time")},
        {QStringLiteral("结束时间"), QStringLiteral("End Time")},
        {QStringLiteral("参数"), QStringLiteral("Metrics")},
        {QStringLiteral("温度"), QStringLiteral("Temperature")},
        {QStringLiteral("湿度"), QStringLiteral("Humidity")},
        {QStringLiteral("查询"), QStringLiteral("Query")},
        {QStringLiteral("时间"), QStringLiteral("Time")},
        {QStringLiteral("最小值"), QStringLiteral("Min")},
        {QStringLiteral("最大值"), QStringLiteral("Max")},
        {QStringLiteral("平均值"), QStringLiteral("Average")},
        {QStringLiteral("统计结果"), QStringLiteral("Statistics")},
        {QStringLiteral("查询记录: 0"), QStringLiteral("Records: 0")},
        {QStringLiteral("查询记录: %1 (最多显示%2条)"), QStringLiteral("Records: %1 (showing up to %2)")},
        {QStringLiteral("配置异常阈值与冷却周期，系统将在实时采集阶段自动判定并记录报警。"), QStringLiteral("Configure thresholds and cooldown. The system will evaluate and record alarms during realtime collection.")},
        {QStringLiteral("阈值设置"), QStringLiteral("Threshold Settings")},
        {QStringLiteral("温度报警"), QStringLiteral("Temperature Alarm")},
        {QStringLiteral("湿度报警"), QStringLiteral("Humidity Alarm")},
        {QStringLiteral("PM2.5报警"), QStringLiteral("PM2.5 Alarm")},
        {QStringLiteral("CO2报警"), QStringLiteral("CO2 Alarm")},
        {QStringLiteral("启用报警"), QStringLiteral("Enable Alarms")},
        {QStringLiteral("报警间隔"), QStringLiteral("Alarm Interval")},
        {QStringLiteral(" 秒"), QStringLiteral(" s")},
        {QStringLiteral("保存设置"), QStringLiteral("Save Settings")},
        {QStringLiteral("报警记录"), QStringLiteral("Alarm Records")},
        {QStringLiteral("开始"), QStringLiteral("Start")},
        {QStringLiteral("结束"), QStringLiteral("End")},
        {QStringLiteral("刷新"), QStringLiteral("Refresh")},
        {QStringLiteral("清空"), QStringLiteral("Clear")},
        {QStringLiteral("值"), QStringLiteral("Value")},
        {QStringLiteral("阈值"), QStringLiteral("Threshold")},
        {QStringLiteral("将历史数据、统计结果和报警记录导出为 CSV，便于归档和二次分析。"), QStringLiteral("Export history data, statistics and alarm records to CSV for archiving and analysis.")},
        {QStringLiteral("导出CSV"), QStringLiteral("Export CSV")},
        {QStringLiteral("导出参数"), QStringLiteral("Export Metrics")},
        {QStringLiteral("导出统计CSV"), QStringLiteral("Export Statistics CSV")},
        {QStringLiteral("使用上方时间范围"), QStringLiteral("Use the time range above")},
        {QStringLiteral("导出报警记录"), QStringLiteral("Export Alarm Records")},
        {QStringLiteral("统一管理刷新频率、提示方式和数据库维护操作，设置会在下次启动时自动生效。"), QStringLiteral("Manage refresh rate, alerts and database maintenance. Settings are persisted automatically.")},
        {QStringLiteral("主题模式"), QStringLiteral("Theme Mode")},
        {QStringLiteral("主题状态"), QStringLiteral("Theme Status")},
        {QStringLiteral("重新加载主题"), QStringLiteral("Reload Theme")},
        {QStringLiteral("报警提示"), QStringLiteral("Alarm Sound")},
        {QStringLiteral("提示音"), QStringLiteral("Beep")},
        {QStringLiteral("静音"), QStringLiteral("Mute")},
        {QStringLiteral("应用设置"), QStringLiteral("Apply Settings")},
        {QStringLiteral("数据库维护"), QStringLiteral("Database Maintenance")},
        {QStringLiteral("数据库路径"), QStringLiteral("Database Path")},
        {QStringLiteral("恢复数据库"), QStringLiteral("Restore Database")},
        {QStringLiteral("保留周期"), QStringLiteral("Retention")},
        {QStringLiteral(" 天"), QStringLiteral(" days")},
        {QStringLiteral("清理历史数据"), QStringLiteral("Clean History Data")},
        {QStringLiteral("界面语言"), QStringLiteral("Interface Language")},
        {QStringLiteral("中文"), QStringLiteral("Chinese")},
        {QStringLiteral("当前语言"), QStringLiteral("Current Language")},
        {QStringLiteral("管理员可在此维护用户账号、角色与密码，普通用户不会看到该页面。"), QStringLiteral("Administrators can manage users, roles and passwords here. Regular users cannot see this page.")},
        {QStringLiteral("添加用户"), QStringLiteral("Add User")},
        {QStringLiteral("普通用户"), QStringLiteral("User")},
        {QStringLiteral("管理员"), QStringLiteral("Admin")},
        {QStringLiteral("添加"), QStringLiteral("Add")},
        {QStringLiteral("输入用户名"), QStringLiteral("Enter username")},
        {QStringLiteral("输入初始密码"), QStringLiteral("Enter initial password")},
        {QStringLiteral("角色"), QStringLiteral("Role")},
        {QStringLiteral("ID"), QStringLiteral("ID")},
        {QStringLiteral("创建时间"), QStringLiteral("Created At")},
        {QStringLiteral("删除用户"), QStringLiteral("Delete User")},
        {QStringLiteral("切换权限"), QStringLiteral("Toggle Role")},
        {QStringLiteral("请先选择一个用户"), QStringLiteral("Please select a user first")},
        {QStringLiteral("采样序列"), QStringLiteral("Sample Index")},
        {QStringLiteral("归一化值"), QStringLiteral("Normalized Value")},
        {QStringLiteral("预测"), QStringLiteral("Prediction")},
        {QStringLiteral("(预测)"), QStringLiteral(" (Predicted)")},
        {QStringLiteral("最近%1条采样数据，虚线表示接下来%2条预测"), QStringLiteral("Latest %1 samples, dashed lines show the next %2 predictions")},
        {QStringLiteral("系统默认深色"), QStringLiteral("System Dark")},
        {QStringLiteral("系统默认浅色"), QStringLiteral("System Light")},
        {QStringLiteral("优化浅色"), QStringLiteral("Optimized Light")},
        {QStringLiteral("优化深色"), QStringLiteral("Optimized Dark")},
        {QStringLiteral("浅色"), QStringLiteral("Light")},
        {QStringLiteral("深色"), QStringLiteral("Dark")},
        {QStringLiteral("跟随系统"), QStringLiteral("Follow System")},
        {QStringLiteral("UDP 请求发送失败: %1"), QStringLiteral("Failed to send UDP request: %1")},
        {QStringLiteral("UDP 请求超时，请检查本地服务 %1:%2 是否已启动"), QStringLiteral("UDP request timed out. Please check whether the local service %1:%2 is running")},
        {QStringLiteral("收到的 UDP 数据不是有效 JSON: %1"), QStringLiteral("Received UDP data is not valid JSON: %1")},
        {QStringLiteral("JSON 字段不完整，缺少 temperature/humidity/pm25/co2"), QStringLiteral("Incomplete JSON fields: missing temperature/humidity/pm25/co2")},
        {QStringLiteral("时间,参数,值,阈值"), QStringLiteral("Time,Metric,Value,Threshold")},
        {QStringLiteral("参数,最小值,最大值,平均值"), QStringLiteral("Metric,Min,Max,Average")}
    };
}

const QHash<QString, QString> &englishMap()
{
    static const QHash<QString, QString> map = buildEnglishMap();
    return map;
}
}

AppTranslator::AppTranslator(QObject *parent)
    : QTranslator(parent)
{
}

QString AppTranslator::translate(const char *context, const char *sourceText,
                                 const char *disambiguation, int n) const
{
    Q_UNUSED(context);
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);

    const QString key = QString::fromUtf8(sourceText);
    const auto it = englishMap().constFind(key);
    return it == englishMap().constEnd() ? QString() : it.value();
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
{
}

void LanguageManager::initialize()
{
    QSettings settings;
    setLanguage(languageFromKey(settings.value(QStringLiteral("language"), QStringLiteral("zh")).toString()));
}

LanguageManager::Language LanguageManager::language() const
{
    return currentLanguage_;
}

QString LanguageManager::languageKey() const
{
    return currentLanguage_ == Language::English ? QStringLiteral("en") : QStringLiteral("zh");
}

QString LanguageManager::currentLanguageDisplayName() const
{
    return currentLanguage_ == Language::English ? QStringLiteral("English") : QStringLiteral("中文");
}

QString LanguageManager::nextLanguageButtonText() const
{
    return currentLanguage_ == Language::English
        ? tr("Switch to 中文")
        : tr("切换到 English");
}

void LanguageManager::setLanguage(Language language)
{
    const bool changed = currentLanguage_ != language;
    currentLanguage_ = language;
    applyLanguage();

    QSettings settings;
    settings.setValue(QStringLiteral("language"), languageKey());

    if (changed) {
        emit languageChanged();
    }
}

void LanguageManager::toggleLanguage()
{
    setLanguage(currentLanguage_ == Language::English ? Language::Chinese : Language::English);
}

LanguageManager::Language LanguageManager::languageFromKey(const QString &key)
{
    return key.compare(QStringLiteral("en"), Qt::CaseInsensitive) == 0
        ? Language::English
        : Language::Chinese;
}

void LanguageManager::applyLanguage()
{
    qApp->removeTranslator(&translator_);
    if (currentLanguage_ == Language::English) {
        qApp->installTranslator(&translator_);
    }
}