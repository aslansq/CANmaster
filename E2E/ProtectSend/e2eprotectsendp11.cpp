#include "e2eprotectsendp11.h"
#include "ui_e2eprotectsendp11.h"

E2EProtectSendP11::E2EProtectSendP11(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::E2EProtectSendP11)
{
	ui->setupUi(this);
}

E2EProtectSendP11::~E2EProtectSendP11()
{
	delete ui;
}

E2EProtectSendP11::DataIdMode E2EProtectSendP11::getDataIdMode(void) const
{
	DataIdMode mode;
	QString s = ui->dataIdModeComboBox->currentText();

	if(s == "NIBBLE") {
		mode = DataIdMode::NIBBLE;
	} else {
		mode = DataIdMode::BOTH;
	}

	return mode;
}

uint16_t E2EProtectSendP11::getDataId(void) const
{
	return ui->dataIdSpinBox->value();
}


void E2EProtectSendP11::setDataId(uint16_t id)
{
	ui->dataIdSpinBox->setValue((int)id);
}

void E2EProtectSendP11::setDataIdMode(DataIdMode mode)
{
	ui->dataIdModeComboBox->setCurrentIndex(static_cast<int>(mode));
}

void E2EProtectSendP11::setDisabled(bool isDisabled)
{
	ui->dataIdSpinBox->setDisabled(isDisabled);
	ui->dataIdModeComboBox->setDisabled(isDisabled);
}

