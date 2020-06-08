#ifndef AUTORUNS_H
#define AUTORUNS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class AutoRuns; }
QT_END_NAMESPACE

class AutoRuns : public QMainWindow
{
    Q_OBJECT

public:
    AutoRuns(QWidget *parent = nullptr);
    ~AutoRuns();

private:
    Ui::AutoRuns *ui;
};
#endif // AUTORUNS_H
