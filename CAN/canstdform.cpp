#include "canstdform.h"
#include "ui_canstdform.h"

CanStdForm::CanStdForm(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::CanStdForm)
	, baud("1 MBit/s")
	, channel("PCAN_USBBUS1")
	,availableBaudrates({
		{"1 MBit/s"  , 1000000},
		{"800 kBit/s", 800000 },
		{"500 kBit/s", 500000 },
		{"250 kBit/s", 250000 },
		{"125 kBit/s", 125000 },
		{"100 kBit/s", 100000 },
		{"95 kBit/s" , 95000  },
		{"83 kBit/s" , 83000  },
		{"50 kBit/s" , 50000  },
		{"47 kBit/s" , 47000  },
		{"33 kBit/s" , 33000  },
		{"20 kBit/s" , 20000  },
		{"10 kBit/s" , 10000  },
		{"5 kBit/s"  , 5000   }
	})
	,availableDevChannels({
		{"PCAN_USBBUS1", 1},
		{"PCAN_USBBUS2", 2},
		{"PCAN_USBBUS3", 3},
		{"PCAN_USBBUS4", 4},
		{"PCAN_USBBUS5", 5},
		{"PCAN_USBBUS6", 6}
	})
{
	ui->setupUi(this);
	for(const QString &key : availableDevChannels.keys()) {
		ui->channelComboBox->addItem(key);
	}
	for(const QString &key : availableBaudrates.keys()) {
		ui->baudComboBox->addItem(key);
	}
}

CanStdForm::~CanStdForm()
{
	delete ui;
}

uint64_t CanStdForm::getBaud(void) const
{
	return availableBaudrates[baud];
}
int CanStdForm::getChannel(void) const
{
	return availableDevChannels[channel];
}

QString CanStdForm::getInterface(void)
{
	return "PEAK";
}

void CanStdForm::setDisabled(bool disabled)
{
	ui->baudComboBox->setDisabled(disabled);
	ui->channelComboBox->setDisabled(disabled);
}

void CanStdForm::on_channelComboBox_currentTextChanged(const QString &arg1)
{
	channel = arg1;
}


void CanStdForm::on_baudComboBox_currentTextChanged(const QString &arg1)
{
	baud = arg1;
}

