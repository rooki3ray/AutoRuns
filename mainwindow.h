#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <windows.h>
#include <QMainWindow>
#include <QSettings>
#include <QFileInfo>
#include <QPushButton>
#include <QFileIconProvider>
#include <QDateTime>
#include <QProcess>
#include <Qt>
#include <QTableWidgetItem>
#include <QTextCodec>
QT_BEGIN_NAMESPACE
namespace Ui { class AutoRuns; }
QT_END_NAMESPACE

class AutoRuns : public QMainWindow
{
    Q_OBJECT

public:
    AutoRuns(QWidget *parent = nullptr);
    ~AutoRuns();

private slots:

    void on_Logon_clicked();

    void on_Services_clicked();

    void on_Drivers_clicked();

    void on_Tasks_clicked();

    void on_Knowndlls_clicked();

    void on_tableWidget_logon_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_services_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_drivers_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_tasks_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_knowndlls_itemClicked(QTableWidgetItem *item);

    // 自定义函数
    void initTableColumnWidth(QTableWidget* t);

    void SetTableItem(QTableWidget* t, QString str, QString value, int *rowIndex);

    void initTableLogon(QTableWidget* t, HKEY hKey, QString PATH);

    void initTableServiceDrivers(QTableWidget* t_services, QTableWidget* t_drivers, HKEY hKey, QString PATH);

    void initTableTasks(QTableWidget* t);

    void initTableKnowndlls(QTableWidget* t, HKEY hKey, QString PATH);

    void imagePath_std(QString* value);

    void btn_table_switch(int mode);

    void show_info(QTableWidget* t, QTableWidgetItem *item);

private:
    Ui::AutoRuns *ui;
    int state = 2;   //表示当前所在状态
    QPushButton* button[7];
};
#endif // MAINWINDOW_H
