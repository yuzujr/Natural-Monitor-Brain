# 环境数据信息平台（Qt）

这是一个基于 Qt Widgets 的环境监测教学项目，覆盖用户登录、环境数据采集、历史统计、异常报警、CSV 导出和系统设置等完整流程。项目当前以模拟传感器数据驱动，也已经具备继续接入 TCP/UDP 实时数据源的结构基础。

## 当前特性
- 登录、注册、找回密码与管理员默认账号初始化
- 注销当前会话后返回登录界面，无需重启程序
- 管理员用户维护：新增、删除、角色切换、密码重置
- 实时数据展示：温度、湿度、PM2.5、CO2 指标卡片与趋势图
- 历史数据查询与统计：按时间范围查询，展示最小值、最大值、平均值
- 异常报警：阈值配置、冷却时间、防重复触发、报警记录持久化
- CSV 导出：历史数据、统计结果、报警记录
- 系统设置：刷新频率、提示音、浅色/深色/跟随系统主题切换、数据库备份恢复、历史清理
- 主题切换只调整调色板，不切换控件风格，避免切换后布局尺寸发生变化

## 项目结构
- src/pages：各功能页 QWidget，负责界面与页内交互
- src/services：跨页面业务能力，如报警判定、CSV 导出
- src/repositories：用户、采样、报警数据访问层
- src/common：共享类型与统一样式
- src/databasemanager.*：数据库连接、建表、仓储暴露
- src/mainwindow.*：主窗口装配与跨模块协调

## 源码文件说明

### 根目录
- CMakeLists.txt：项目构建入口，定义 Qt Widgets/Sql 依赖、源文件列表、安装规则和可执行程序目标。

### src 目录
- src/main.cpp：程序入口，负责创建 QApplication、初始化主题管理器和数据库、拉起登录流程，并在注销后重新回到登录界面。
- src/mainwindow.h：主窗口类声明，定义页面装配、数据采样、导出、报警刷新、用户管理和注销等协调接口。
- src/mainwindow.cpp：主窗口实现，负责组装各个页面、连接信号槽、定时生成模拟数据、刷新历史/报警记录、响应设置变更。
- src/logindialog.h：登录对话框声明，定义登录、注册、找回密码相关控件和当前登录用户信息。
- src/logindialog.cpp：登录对话框实现，负责用户认证、注册新用户、重置密码，以及登录页主题样式应用。
- src/databasemanager.h：数据库管理器声明，定义数据库连接、建表初始化、默认管理员初始化和仓储对象访问接口。
- src/databasemanager.cpp：数据库管理器实现，负责 SQLite 打开/关闭/重连、错误信息维护、密码哈希和 schema 初始化。
- src/themeutils.h：主题管理器声明，定义浅色、深色、跟随系统三种主题模式及其查询、切换接口。
- src/themeutils.cpp：主题管理器实现，负责固定控件风格、应用调色板、记录当前主题状态并触发全局主题刷新。
- src/linechartwidget.h：折线图控件声明，用于展示实时环境数据趋势。
- src/linechartwidget.cpp：折线图控件实现，负责绘制温度、湿度、PM2.5、CO2 的历史曲线。

### src/common
- src/common/moduletypes.h：共享数据结构定义，包括用户信息、环境样本、统计结果、报警设置、报警事件和导出筛选项。
- src/common/uistyles.h：统一样式接口声明，提供页面、对话框、主窗口、标签页、按钮和指标卡片样式函数。
- src/common/uistyles.cpp：统一样式实现，负责深浅主题文字颜色、卡片样式、按钮分级、日期日历弹窗样式等视觉规则。

### src/pages
- src/pages/realtimepagewidget.h：实时数据页声明，定义实时指标卡片、刷新频率控制、模拟采集开关和趋势图接口。
- src/pages/realtimepagewidget.cpp：实时数据页实现，负责展示当前环境指标、更新时间、趋势图，并在主题切换时重新应用卡片样式。
- src/pages/historypagewidget.h：历史查询页声明，定义时间范围查询、参数筛选和统计结果展示接口。
- src/pages/historypagewidget.cpp：历史查询页实现，负责展示历史数据表和统计结果表。
- src/pages/alarmpagewidget.h：报警页声明，定义阈值设置、报警记录查询、默认时间范围和时间跟随更新接口。
- src/pages/alarmpagewidget.cpp：报警页实现，负责展示报警阈值配置、报警记录表，并维护报警查询的开始/结束时间。
- src/pages/exportpagewidget.h：导出页声明，定义历史数据、统计结果、报警记录三类导出请求信号。
- src/pages/exportpagewidget.cpp：导出页实现，负责采集导出时间范围与参数筛选条件并发出导出请求。
- src/pages/settingspagewidget.h：系统设置页声明，定义刷新频率、主题模式、报警提示、数据库维护相关接口。
- src/pages/settingspagewidget.cpp：系统设置页实现，负责主题模式切换、备份恢复数据库、历史清理和设置应用。
- src/pages/usermanagementpagewidget.h：用户管理页声明，定义新增用户、删除用户、切换角色、重置密码等操作信号。
- src/pages/usermanagementpagewidget.cpp：用户管理页实现，负责展示用户表格并提供管理员操作入口。

### src/repositories
- src/repositories/userrepository.h：用户仓储声明，定义用户验证、创建、密码修改、角色更新、删除和列表查询接口。
- src/repositories/userrepository.cpp：用户仓储实现，封装 user 表相关 SQL 操作。
- src/repositories/samplerepository.h：采样仓储声明，定义历史样本写入、时间范围查询、统计和旧数据清理接口。
- src/repositories/samplerepository.cpp：采样仓储实现，封装环境数据表的插入、聚合统计和清理操作。
- src/repositories/alarmrepository.h：报警仓储声明，定义报警记录写入、范围查询和清空接口。
- src/repositories/alarmrepository.cpp：报警仓储实现，封装 alarm 表相关 SQL 操作。

### src/services
- src/services/alarmservice.h：报警服务声明，定义阈值判断和冷却时间控制接口。
- src/services/alarmservice.cpp：报警服务实现，负责判断样本是否越限、写入报警记录并返回报警事件列表。
- src/services/csvexporter.h：CSV 导出服务声明，定义历史数据、统计结果、报警记录导出接口。
- src/services/csvexporter.cpp：CSV 导出服务实现，负责将查询结果写出为 CSV 文件。

## 已完成的精简
- 删除了仅用于创建空中央部件的 MainWindow UI 文件，主窗口改为纯代码构建
- 移除了 DatabaseManager 中与仓储层重复的 CRUD 转发方法
- 报警判定逻辑已从 MainWindow 提取到 AlarmService
- 页面样式已统一到公共样式模块，减少重复样式代码，并支持主按钮/次按钮/危险按钮分级

## 运行说明
- 默认管理员账号：admin / admin123
- 数据库文件保存在 Qt AppDataLocation 对应目录
- 首次启动会自动创建数据库表结构与默认管理员账号

## 构建方式

### Qt Creator
直接打开 CMakeLists.txt，选择 Qt 6 或 Qt 5 Kits 后构建运行。

### 命令行
在已生成的构建目录中执行：

```powershell
cmake --build build/Desktop_Qt_6_9_3_llvm_mingw_64_bit-Debug
```

## Windows 打包

给其他 Windows 用户分发时，不要只发 exe，需要把 Qt 运行时一起部署。当前项目最直接的方式是使用 Qt 自带的 windeployqt。

1. 先正常构建项目，得到 build/Desktop_Qt_6_9_3_llvm_mingw_64_bit-Debug/Natural-Monitor-Brain.exe。
2. 在 Qt 终端或已配置好 Qt 环境变量的 PowerShell 中执行：

```powershell
windeployqt --qmldir src build/Desktop_Qt_6_9_3_llvm_mingw_64_bit-Debug/Natural-Monitor-Brain.exe
```

3. 将以下内容一起打包发给对方：
- Natural-Monitor-Brain.exe
- windeployqt 复制出来的 platforms、styles、iconengines 等 Qt 依赖目录
- 同目录下生成的 Qt6Core.dll、Qt6Gui.dll、Qt6Widgets.dll、Qt6Sql.dll 等动态库

4. 最后把整个 build/Desktop_Qt_6_9_3_llvm_mingw_64_bit-Debug 目录压缩成 zip，对方解压后直接运行 exe 即可。

如果目标机器没有 Visual C++ 运行库或 MinGW 运行库，还需要一并补齐对应编译器运行时。当前你使用的是 Qt 6.9.3 llvm-mingw 套件，最稳妥的做法就是在一台未安装 Qt 的 Windows 机器上先试运行一次压缩包。

## 后续可扩展方向
- 接入 TCP/UDP 或串口采集服务，替换当前模拟数据源
- 为历史查询和报警记录增加分页、排序和更细粒度筛选
- 引入 Excel 导出库
