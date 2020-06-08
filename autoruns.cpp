#include "autoruns.h"
#include "ui_autoruns.h"

AutoRuns::AutoRuns(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AutoRuns)
{
    ui->setupUi(this);
}

AutoRuns::~AutoRuns()
{
    delete ui;
}

