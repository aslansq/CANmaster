#include "e2ereceivecheckp11.h"
#include "ui_e2ereceivecheckp11.h"

E2EReceiveCheckP11::E2EReceiveCheckP11(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::E2EReceiveCheckP11)
{
	ui->setupUi(this);
}

E2EReceiveCheckP11::~E2EReceiveCheckP11()
{
	delete ui;
}

E2EReceiveCheckP11::DataIdMode E2EReceiveCheckP11::getDataIdMode(void) const
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

uint16_t E2EReceiveCheckP11::getDataId(void) const
{
	return ui->dataIdSpinBox->value();
}

uint32_t E2EReceiveCheckP11::getMaxDeltaCounter(void) const
{
	return ui->maxDeltaSpinBox->value();
}

void E2EReceiveCheckP11::setDataId(uint16_t id)
{
	ui->dataIdSpinBox->setValue(static_cast<int>(id));
}

void E2EReceiveCheckP11::setDataIdMode(DataIdMode mode)
{
	ui->dataIdModeComboBox->setCurrentIndex(static_cast<int>(mode));
}


void E2EReceiveCheckP11::on_dataIdSpinBox_valueChanged(int arg1)
{
	(void)arg1;
	emit configChanged();
}


void E2EReceiveCheckP11::on_dataIdModeComboBox_currentTextChanged(const QString &arg1)
{
	(void)arg1;
	emit configChanged();
}


void E2EReceiveCheckP11::on_maxDeltaSpinBox_valueChanged(int arg1)
{
	(void)arg1;
	emit configChanged();
}

