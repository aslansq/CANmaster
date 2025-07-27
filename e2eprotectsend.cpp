#include "e2eprotectsend.h"
#include "ui_e2eprotectsend.h"

E2EProtectSend::E2EProtectSend(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::E2EProtectSend)
{
	ui->setupUi(this);
}

E2EProtectSend::~E2EProtectSend()
{
	delete ui;
}

