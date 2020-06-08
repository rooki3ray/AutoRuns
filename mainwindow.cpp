#include "mainwindow.h"
#include "RegRead.h"
#include "Description.h"
#include "CheckSignitures.h"
#include "Publisher.h"
#include "Tasks.h"
#include "FileExists.h"
#include "ui_mainwindow.h"
#define logon { "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",\
                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run",\
                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",\
                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"}
#define services1 "System\\CurrentControlSet\\Services"
#define knownddls1 "System\\CurrentControlSet\\Control\\Session Manager\\KnownDlls"
AutoRuns::AutoRuns(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AutoRuns)
{
    /* 遍历注册表： https://www.iteye.com/blog/tcspecial-1846518
     *           Services自启动：https://www.cnblogs.com/enych/p/10315938.html
     * TableWidget： 增删改： https://www.cnblogs.com/li-peng/p/3654634.html
     *               图标： https://blog.csdn.net/qqwangfan/article/details/51735989
     *               行列操作： https://blog.csdn.net/weixin_43245453/article/details/96591959 */
    ui->setupUi(this);
    button[2] = ui->Logon;
    button[3] = ui->Services;
    button[4] = ui->Drivers;
    button[5] = ui->Tasks;
    button[6] = ui->Knownddls;
    for (int i = 2; i <= 6; i++)
    {
        button[i]->setStyleSheet( "border:none; background-color:transparent;" );
//        button[i]->setFlat(true);
    }
//    KEY_VALUE *list = new KEY_VALUE[2048];
//    int length = 0;
//    QStringList result[2];
//    GetGroupKeyValue(HKLM, services1, list, &length, result);

    ui->icon->setStyleSheet( "background-color:transparent;" );
    button[state]->setStyleSheet( "border:none; background-color:rgb(194,222,245);" );  // 启动时在logon界面，设置相应的按钮状态
    button[state]->setDisabled(true);
    ui->stack->addWidget(ui->tableWidget_logon);
    ui->stack->addWidget(ui->tableWidget_services);
    ui->stack->addWidget(ui->tableWidget_drivers);
    ui->stack->addWidget(ui->tableWidget_tasks);
    ui->stack->addWidget(ui->tableWidget_knownddls);
    ui->stack->setCurrentIndex(2);
    ui->stack->show();
    initTableColumnWidth(ui->tableWidget_logon);
    initTableColumnWidth(ui->tableWidget_services);
    initTableColumnWidth(ui->tableWidget_drivers);
    initTableColumnWidth(ui->tableWidget_tasks);
    initTableColumnWidth(ui->tableWidget_knownddls);

    foreach(QString path, QStringList logon)
    {
        initTableLogon(ui->tableWidget_logon, HKLM, path);
        initTableLogon(ui->tableWidget_logon, HKCU, path);
    }
    initTableServiceDrivers(ui->tableWidget_services, ui->tableWidget_drivers, HKLM, QString(services1));
    initTableTasks(ui->tableWidget_tasks);
    initTableKnownddls(ui->tableWidget_knownddls, HKLM, QString(knownddls1));
}

AutoRuns::~AutoRuns()
{
    delete ui;
}

void AutoRuns::SetTableItem(QTableWidget* t, QString str, QString value, int *rowIndex)
{
//    QProcess process;
//    QString cmd = QString("e:/SysinternalsSuite/sigcheck64.exe \"") + value.toLocal8Bit().data() + QString("\"");
//    process.start(cmd);
//    process.waitForFinished();
//    QByteArray result = process.readAllStandardOutput();
//    QString msg_result = QString::fromLocal8Bit(result);
    t->setRowCount(*rowIndex+1);//总行数增加1
    t->setRowHeight(*rowIndex, 20);
    const wchar_t * filepath = reinterpret_cast<const wchar_t *>(value.utf16());    // 将QString转换为LPCTSTR(const wchar_t *)

    wchar_t filepath2[1024];        // 将QString转换为TCHAR *(wchar_t *)
    wcscpy_s(reinterpret_cast<wchar_t*>(filepath2),
        sizeof(filepath2) / sizeof(wchar_t),
        reinterpret_cast<const wchar_t*>(value.utf16()));

    const char* std_str = value.toStdString().data();
    char buf[4096] = {0};
    strcpy(buf, std_str);
    int exists = fileExists(buf);

    if (exists)
    {
//        qDebug()<<"NOT";
        t->setItem(*rowIndex, 0, new QTableWidgetItem(" "));
        t->setItem(*rowIndex, 1, new QTableWidgetItem(str.toLocal8Bit().data()));
        t->setItem(*rowIndex, 2, new QTableWidgetItem("File Not Found:"+value));
        t->setSpan(*rowIndex, 2, 1, 4);
        t->item(*rowIndex, 2)->setBackgroundColor(QColor(240,240,0));
    }
    else
    {
        QFileInfo file_info(value.toLocal8Bit().data());
        QString time = file_info.created().toString("yyyy-MM-dd hh:mm:ss");
        QFileIconProvider icon_provider;
        QIcon icon = icon_provider.icon(file_info);    //获取图标

        TCHAR* ptszStr = new TCHAR[1024];   //获取文件描述
        bool dessuc = GetFileVersionString(filepath, _T("FileDescription"), ptszStr, 1024);
        QString description;
        description = (dessuc) ? QString::fromWCharArray(ptszStr) : "----------";

        bool Verified = VerifyEmbeddedSignature(filepath);  //获取签名信息
        QString Sig = (Verified)? "(Verified) " : "(Not Verified) ";

        LPTSTR ptStr = NULL;
        bool pubsuc = Publisher(filepath2, &ptStr);
        QString publisher;
        publisher = (pubsuc) ? QString::fromWCharArray(ptStr) : "-----------";
//        _tprintf(_T("\n------------Subject Name: %s\n"), ptStr);
//        qDebug()<<value<<publisher;

        t->setItem(*rowIndex, 0, new QTableWidgetItem(icon, " "));
        t->setItem(*rowIndex, 1, new QTableWidgetItem(str.toLocal8Bit().data()));
        t->setItem(*rowIndex, 2, new QTableWidgetItem(description.toLocal8Bit().data()));
        t->setItem(*rowIndex, 3, new QTableWidgetItem((Sig + publisher).toLocal8Bit().data()));
        t->setItem(*rowIndex, 4, new QTableWidgetItem(value.toLocal8Bit().data()));
        t->setItem(*rowIndex, 5, new QTableWidgetItem(time));

        if (!Verified)
            for (int i = 1; i <= 5; i++)
               t->item(*rowIndex, i)->setBackgroundColor(QColor(255,208,208));
        delete [] ptszStr;
    }
    *rowIndex += 1;
}

void AutoRuns::initTableColumnWidth(QTableWidget* t)
{
    //初始化QTableWidget的列宽
    t->setColumnWidth(0,20);
    t->setColumnWidth(1,145);
    t->setColumnWidth(2,210);
    t->setColumnWidth(3,200);
    t->setColumnWidth(4,420);
    t->setColumnWidth(5,200);
}

void AutoRuns::initTableLogon(QTableWidget* t, HKEY hKey, QString PATH)
{
    KEY_VALUE *list = new KEY_VALUE[100];
    int length = 0;
    QStringList result[2];

    GetKeyValue(hKey, PATH.toLocal8Bit(), list, &length);
    for (int i = 0; i < length; i++)
    {
        QString key = "";
        QString value = "";
        for(int j = 0; j < list[i].keylen; j++)
        {
            key += (char)list[i].key[j];
        }
        result[0]<<key;
        for(int j = 0; j < list[i].valuelen; j=j+2)
        {
            value += ((char)list[i].value[j]);
        }
        result[1]<<value;
    }
    int rowIndex = t->rowCount();   // 获取QTableWidget的总行数
//    QSettings *settings=new QSettings(PATH, QSettings::NativeFormat);
    QString Hk = (hKey==HKLM) ? "HKLM\\" : "HKCU\\";
    QTableWidgetItem *item = new QTableWidgetItem(Hk + PATH);
    bool flag = 0;
    foreach(QString str, result[0])
//    foreach(QString str,settings->childKeys())
    {
         if (!flag)     //对每个注册表目录新建一行，显示路径
         {
             item ->setBackgroundColor(QColor(208,208,255));
             t->setRowCount(rowIndex+1);//总行数增加1
             t->setItem(rowIndex, 0, item);
             t->setSpan(rowIndex++, 0, 1, 6);
             flag = 1;
         }
         QString value = result[1][result[0].indexOf(str)];         //Qt方法则改为：QString value=settings->value(str).toString();
         imagePath_std(&value);
         SetTableItem(t, str, value, &rowIndex);
         t->selectRow(0);
    }
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setVisible(false);
    delete [] list;
}

void AutoRuns::initTableServiceDrivers(QTableWidget* t_services, QTableWidget* t_drivers, HKEY hKey, QString PATH)
{
    KEY_VALUE *list = new KEY_VALUE[2048];
    int length = 0;
    QStringList result[2];
    GetGroupKeyValue(hKey, PATH.toLocal8Bit(), list, &length);
    for (int i = 0; i < length; i++)
    {
        QString key = "";
        QString value = "";
        for(int j = 0; j < list[i].keylen; j++)
        {
            key += (char)list[i].key[j];
        }
        result[0]<<key;
        for(int j = 0; j < list[i].valuelen; j=j+2)
        {
            value += ((char)list[i].value[j]);
        }
        result[1]<<value;
//        qDebug()<<i<<key.toLocal8Bit()<<value.toLocal8Bit().data();
    }
    int rowIndex_s = t_services->rowCount();
    int rowIndex_d = t_drivers->rowCount();
//    QSettings *settings=new QSettings(PATH, QSettings::NativeFormat);
    QString Hk = (hKey==HKLM) ? "HKLM\\" : "HKCU\\";
    QTableWidgetItem *item = new QTableWidgetItem(Hk + PATH);
    item->setBackgroundColor(QColor(208,208,255));
    QTableWidgetItem *item2 = new QTableWidgetItem(Hk + PATH);
    item2->setBackgroundColor(QColor(208,208,255));
    t_services->setRowCount(rowIndex_s+1);//总行数增加1
    t_services->setItem(rowIndex_s, 0, item);
    t_services->setSpan(rowIndex_s++, 0, 1, 6);
    t_drivers->setRowCount(rowIndex_d+1);//总行数增加1
    t_drivers->setItem(rowIndex_d, 0, item2);
    t_drivers->setSpan(rowIndex_d++, 0, 1, 6);
    foreach(QString group, result[0])
    {
//        qDebug()<<group;
        QString value = result[1][result[0].indexOf(group)];         //Qt方法则改为：QString v=settings->value(str).toString();
        if(!value.contains("ERROR!"))
        {
            imagePath_std(&value);
            QTableWidget* t;
            int* rowIndex;
            if (value.contains("system32\\drivers\\", Qt::CaseInsensitive))
            {
                t = t_drivers;
                rowIndex = &rowIndex_d;
            }
            else
            {
                t = t_services;
                rowIndex = &rowIndex_s;
            }
            SetTableItem(t, group, value, rowIndex);
            t->selectRow(0);
        }
    }
//    foreach(QString group,settings->childGroups())
//    {   // 遍历该注册表的子目录
//        settings->beginGroup(group);
//        int Start = settings->value("Start", -1).toInt();
//        if (Start == 0 || Start == 1 || Start == 2)
//        {
//            QString value = settings->value("ImagePath", "ERROR!").toString();
//            if(!value.contains("ERROR!"))
//            {
//                value_std(&value);
//                QTableWidget* t;
//                int* rowIndex;
//                if (value.contains("system32\\drivers\\", Qt::CaseInsensitive))
//                {
//                    t = t_drivers;
//                    rowIndex = &rowIndex_d;
//                }
//                else
//                {
//                    t = t_services;
//                    rowIndex = &rowIndex_s;
//                }
//                t->setRowCount(*rowIndex+1);//总行数增加1
//                SetTableItem(t, group, value, *rowIndex);
//                t->selectRow(0);
//                *rowIndex += 1;
//            }
//        }
//        settings->endGroup();
//    }
    t_services->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t_services->verticalHeader()->setVisible(false);
    t_drivers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t_drivers->verticalHeader()->setVisible(false);
    delete [] list;
}

void AutoRuns::initTableTasks(QTableWidget* t)
{
    int rowIndex = t->rowCount();
//    QSettings *settings=new QSettings(PATH, QSettings::NativeFormat);
    QString name = "Scheduled Tasks";
    QTableWidgetItem *item = new QTableWidgetItem(name);
    item->setBackgroundColor(QColor(208,208,255));
    t->setRowCount(rowIndex+1);//总行数增加1
    t->setItem(rowIndex, 0, item);
    t->setSpan(rowIndex++, 0, 1, 6);

    tasks* list = new tasks[2048];
    int length = 0;
    allTasks(list, &length);
    for (int i = 0; i < length; i++)
    {
        QString folderName = QString::fromLocal8Bit(list[i].folderName);
        while (folderName.contains("\\\\"))    //部分注册表value/task ImagePath以\开头,需要剔除
            folderName = folderName.replace("\\\\","\\");
        QString taskName = QString::fromLocal8Bit(list[i].taskName);
        QString imagePath = QString::fromLocal8Bit(list[i].imagePath);
//        qDebug()<<(folderName+"\\"+taskName).toLocal8Bit().data()<<imagePath.toLocal8Bit().data();

        imagePath_std(&imagePath);
        SetTableItem(t, folderName+"\\"+taskName, imagePath, &rowIndex);
//        printf("%s\t%s\t%s\n", list[i].folderName, list[i].taskName, list[i].imagePath);
    }
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setVisible(false);
    delete [] list;
}

void AutoRuns::initTableKnownddls(QTableWidget* t, HKEY hKey, QString PATH)
{
    KEY_VALUE *list = new KEY_VALUE[100];
    int length = 0;
    QStringList result[2];
    GetKeyValue(hKey, PATH.toLocal8Bit(), list, &length);
    for (int i = 0; i < length; i++)
    {
        QString key = "";
        QString value = "";
        for(int j = 0; j < list[i].keylen; j++)
        {
            key += (char)list[i].key[j];
        }
        result[0]<<key;
        for(int j = 0; j < list[i].valuelen; j=j+2)
        {
            value += ((char)list[i].value[j]);
        }
        result[1]<<value;
    }
    int rowIndex = t->rowCount();   // 获取QTableWidget的总行数
//    QSettings *settings=new QSettings(PATH, QSettings::NativeFormat);
    QString Hk = (hKey==HKLM) ? "HKLM\\" : "HKCU\\";
    QTableWidgetItem *item = new QTableWidgetItem(Hk + PATH);
    bool flag = 0;
    QString value[2];
    foreach(QString str, result[0])
//    foreach(QString str,settings->childKeys())
    {
         if (!flag)     //对每个注册表目录新建一行，显示路径
         {
             item ->setBackgroundColor(QColor(208,208,255));
             t->setRowCount(rowIndex+1);//总行数增加1
             t->setItem(rowIndex, 0, item);
             t->setSpan(rowIndex++, 0, 1, 6);
             flag = 1;
         }
         QString v = result[1][result[0].indexOf(str)];         //Qt方法则改为：QString v=settings->value(str).toString();
         value[0] = "C:\\Windows\\system32\\"+v;
         value[1] = "C:\\Windows\\SysWOW64\\"+v;
         for (int i=0; i<2; i++)
         {
             SetTableItem(t, str, value[i], &rowIndex);
             t->selectRow(0);
         }
    }
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setVisible(false);
    delete [] list;
}

void AutoRuns::imagePath_std(QString* value)
{
    /*注册表value标准化：
        注册表中的value可能会带有命令参数、系统变量等，不能直接读取对应的文件
        该函数将其标准化为一个文件路径（QString）*/
//    if (value->length()==0)
//        return;
    if (value->indexOf(" -")>=0)    //部分注册表value/task ImagePath后跟有 -[command],需要剔除
        *value = value->split(" -")[0];
    if (value->indexOf(" /")>=0)    //部分注册表value/task ImagePath后跟有 /[command],需要剔除
        *value = value->split(" /")[0];
    if(value->indexOf("\"")>=0)     //部分注册表value/task ImagePath已被""包括
        *value = value->split("\"")[1];
    if(value->contains("%systemroot%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%systemroot%", 0, Qt::CaseInsensitive), 12, "C:\\Windows");
    if(value->contains("\\systemroot", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\systemroot", 0, Qt::CaseInsensitive), 11, "C:\\Windows");
    if(value->contains("%windir%", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("%windir%", 0, Qt::CaseInsensitive), 8, "C:\\Windows");
    if(value->contains("\\??\\", Qt::CaseInsensitive))
        *value = value->replace(value->indexOf("\\??\\"), 4, "");
    if(!value->contains(":"))
    {
        if(value->contains("system32", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("system32", 0, Qt::CaseInsensitive), 8, "c:\\Windows\\System32");
        if(value->contains("syswow64", Qt::CaseInsensitive))
            *value = value->replace(value->indexOf("syswow64", 0, Qt::CaseInsensitive), 8, "c:\\Windows\\SysWOW64");
    }
}

void AutoRuns::on_Logon_clicked()
{
    btn_table_switch(2);
}

void AutoRuns::on_Services_clicked()
{
    btn_table_switch(3);
}

void AutoRuns::on_Drivers_clicked()
{
    btn_table_switch(4);
}

void AutoRuns::on_Tasks_clicked()
{
    btn_table_switch(5);
}
void AutoRuns::on_Knownddls_clicked()
{
    btn_table_switch(6);
}

void AutoRuns::btn_table_switch(int mode)
{
    if (state != mode)
    {
        button[state]->setStyleSheet( "border:none; background-color:transparent;" );
        button[state]->setEnabled(true);
        state = mode;
        button[state]->setStyleSheet( "border:none; background-color:rgb(194,222,245);" );
        button[state]->setDisabled(true);
        ui->stack->setCurrentIndex(state);
    }
}

void AutoRuns::on_tableWidget_logon_itemClicked(QTableWidgetItem *item)
{
    show_info(ui->tableWidget_logon, item);
}

void AutoRuns::on_tableWidget_services_itemClicked(QTableWidgetItem *item)
{
    show_info(ui->tableWidget_services, item);
}

void AutoRuns::on_tableWidget_drivers_itemClicked(QTableWidgetItem *item)
{
    show_info(ui->tableWidget_drivers, item);
}

void AutoRuns::on_tableWidget_knownddls_itemClicked(QTableWidgetItem *item)
{
    show_info(ui->tableWidget_knownddls, item);
}

void AutoRuns::on_tableWidget_tasks_itemClicked(QTableWidgetItem *item)
{
    show_info(ui->tableWidget_tasks, item);
}
void AutoRuns::show_info(QTableWidget* t, QTableWidgetItem *item)
{
    int row = item->row();
    if(!t->item(row,0)->text().contains("HK") && !t->item(row,0)->text().contains("Scheduled") && !t->item(row,2)->text().contains("Found"))
    {
            QFileInfo file_info(t->item(row,4)->text());
            qint64 size = file_info.size();
            int flag = 0;
            while (size>1024)
            {
                size /= 1024;
                flag += 1;
            }
            QString b = tr("%1").arg(size)+(flag==0 ? " Bytes" : (flag==1 ? " KB" : " MB"));
            ui->icon->setIcon(t->item(row,0)->icon());
            ui->name->setText(t->item(row,1)->text());
            ui->description->setText(t->item(row,2)->text());
            ui->company->setText(t->item(row,3)->text());
            ui->path->setText(t->item(row,4)->text());
            ui->size->setText("Size:    "+b);
            ui->time->setText("Time:    "+t->item(row,5)->text());
    }
}




