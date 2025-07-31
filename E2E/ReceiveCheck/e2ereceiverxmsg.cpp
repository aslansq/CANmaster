#include "e2ereceiverxmsg.h"
#include "ui_e2ereceiverxmsg.h"

// needed to make placeholder
E2EReceiveRxMsg::E2EReceiveRxMsg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::E2EReceiveRxMsg)
    , msgIdx(-1)
    , id(0)
    , cycleTime(0.0)
    , dlc(-1)
    , protectProfileStr("NONE")
{
	ui->setupUi(this);
}

// Constructor that initializes the message with a network pointer, name, and message index
E2EReceiveRxMsg::E2EReceiveRxMsg(
	std::shared_ptr<dbcppp::INetwork> netPtr,
	QString name,
	int msgIdx
) :
	ui(new Ui::E2EReceiveRxMsg)
	, msgIdx(msgIdx)
	, id(0)
	, cycleTime(0.0)
	, dlc(-1)
	, protectProfileStr("NONE")
	, p11StatusMap({
		{E2E::P11Status::NO_NEW_DATA, "NO_NEW_DATA"},
		{E2E::P11Status::ERROR, "ERROR"},
		{E2E::P11Status::WRONG_SEQ, "WRONG_SEQ"},
		{E2E::P11Status::WRONG_INPUT, "WRONG_INPUT"},
		{E2E::P11Status::REPEATED, "REPEATED"},
		{E2E::P11Status::OK_SOME_LOST, "OK_SOME_LOST"},
		{E2E::P11Status::OK, "OK"}
	})
{
	ui->setupUi(this);
	this->netPtr = netPtr;
	this->nameTableItem.setText(name);
	this->statusTableItem.setText("NO_NEW_DATA");

	ui->nameLabel->setText(name);
	const dbcppp::IMessage &msgRef = netPtr->Messages_Get(msgIdx);
	id = msgRef.Id();
	dlc = msgRef.MessageSize();
	canMsg.id = id;
	canMsg.isExtd = (id > 0x7FF);
	canMsg.dataLength = dlc;
	memset(canMsg.data, 0, sizeof(canMsg.data));
	canMsg.timestamp = 0;

	updateLastReadDataLabel();
	tryFindMsgCycle();

	ui->idLabel->setText("0x" + QString::number(id, 16).toUpper());
	ui->cycleTimeLabel->setText(QString::number(cycleTime));
	ui->dlcLabel->setText(QString::number(dlc));

	ui->protectionConfigVerticalLayout->addWidget(&p11Widget);
	p11Widget.setVisible(false);

	for(const dbcppp::ISignal &signal : msgRef.Signals()) {
		QString sigName = QString::fromStdString(signal.Name());
		ui->crcComboBox->addItem(sigName);
		ui->countComboBox->addItem(sigName);
	}

	tryFindCrcAndCounter();
	tryFindE2ECfg();
}

E2EReceiveRxMsg::~E2EReceiveRxMsg()
{
	delete ui;
}

void E2EReceiveRxMsg::onCanMsgReceived(const CanMsg &canMsgRef)
{
	canMsg = canMsgRef;
	std::vector<uint8_t> frame;
	for(int i = 0; i < canMsgRef.dataLength; ++i) {
		frame.push_back(canMsgRef.data[i]);
	}
	E2E::P11Status status = p11Ptr->check(frame);
	statusTableItem.setText(p11StatusMap[status]);

	updateLastReadDataLabel();
}

void E2EReceiveRxMsg::tryFindMsgCycle(void)
{
	const dbcppp::IMessage &msgRef = netPtr->Messages_Get(msgIdx);
	for(int i = 0; i < msgRef.AttributeValues_Size(); ++i) {
		const dbcppp::IAttribute& attr = msgRef.AttributeValues_Get(i);
		dbcppp::IAttribute::value_t value = attr.Value();
		if(attr.Name() != "GenMsgCycleTime") {
			continue;
		}
		cycleTime = std::get<double>(value);
	}
}

void E2EReceiveRxMsg::tryFindE2ECfg(void)
{
	const dbcppp::IMessage &msgRef = netPtr->Messages_Get(msgIdx);
	for(int i = 0; i < msgRef.AttributeValues_Size(); ++i) {
		const dbcppp::IAttribute& attr = msgRef.AttributeValues_Get(i);
		dbcppp::IAttribute::value_t value = attr.Value();
		QString attrStr = QString::fromStdString(attr.Name());
		if(attrStr.contains("E2EDataId", Qt::CaseInsensitive)) {
			double dataId = std::get<double>(value);
			p11Widget.setDataId(dataId);
			break;
		}
	}

	for(int i = 0; i < msgRef.AttributeValues_Size(); ++i) {
		const dbcppp::IAttribute& attr = msgRef.AttributeValues_Get(i);
		dbcppp::IAttribute::value_t value = attr.Value();
		QString attrStr = QString::fromStdString(attr.Name());
		if(attrStr.contains("E2EProfile", Qt::CaseInsensitive)) {
			std::string profileStr = std::get<std::string>(value);
			QString profileQStr = QString::fromStdString(profileStr);
			if(profileQStr.contains("P11", Qt::CaseInsensitive)) {
				ui->protectionComboBox->setCurrentText("P11");
			}
			break;
		}
	}
}

void E2EReceiveRxMsg::tryFindCrcAndCounter(void)
{
	for (int i = 0; i < ui->crcComboBox->count(); ++i) {
		QString text = ui->crcComboBox->itemText(i);
		if (text.contains("crc", Qt::CaseInsensitive)) {
			ui->crcComboBox->setCurrentIndex(i);
			break;
		}
	}

	for (int i = 0; i < ui->countComboBox->count(); ++i) {
		QString text = ui->countComboBox->itemText(i);
		if (text.contains("count", Qt::CaseInsensitive) ||
			text.contains("cnt", Qt::CaseInsensitive)) {
			ui->countComboBox->setCurrentIndex(i);
			break;
		}
	}
}

void E2EReceiveRxMsg::updateLastReadDataLabel(void)
{
	QString lastReadDataStr = "";
	for(int i = 0; i < canMsg.dataLength; ++i) {
		lastReadDataStr += QString::number(canMsg.data[i], 16).toUpper();
		if(i < canMsg.dataLength - 1) {
			lastReadDataStr += " ";
		}
	}
	ui->lastReadDataLabel->setText(lastReadDataStr);
}

uint32_t E2EReceiveRxMsg::getCounterStartBit(void)
{
	QString countSigName = ui->countComboBox->currentText();
	return getStartBit(countSigName);
}

uint32_t E2EReceiveRxMsg::getCrcStartBit(void)
{
	QString crcSigName = ui->crcComboBox->currentText();
	return getStartBit(crcSigName);
}

void E2EReceiveRxMsg::configProtection(void)
{
	if(ui->protectionComboBox->currentText() == "P11") {
		p11ConfigPtr.reset();
		p11Ptr.reset();
		p11ConfigPtr = std::make_unique<E2E::P11Config>();
		p11ConfigPtr->dataId = p11Widget.getDataId();
		p11ConfigPtr->dataLen = dlc * 8; // in bits
		p11ConfigPtr->dataIdMode =
			p11Widget.getDataIdMode() == E2EReceiveCheckP11::DataIdMode::BOTH ?
			E2E::P11DataIdModes::BOTH : E2E::P11DataIdModes::DATA_ID_NIBBLE;
		p11ConfigPtr->counterOffset = getCounterStartBit();
		p11ConfigPtr->crcOffset = getCrcStartBit();
		p11ConfigPtr->dataIdNibbleOffset = getCounterStartBit() + 4;
		p11ConfigPtr->maxDeltaCounter = p11Widget.getMaxDeltaCounter();
		p11Ptr = std::make_unique<E2E::P11>(
			E2E::P11Functionality::CHECK,
			*p11ConfigPtr
		);
	}
}

uint32_t E2EReceiveRxMsg::getStartBit(QString signalName)
{
	const dbcppp::IMessage &msgRef = netPtr->Messages_Get(msgIdx);
	for(const dbcppp::ISignal &signal : msgRef.Signals()) {
		QString sigName = QString::fromStdString(signal.Name());
		if(sigName == signalName) {
			return signal.StartBit();
		}
	}
	return 0;
}

void E2EReceiveRxMsg::on_crcComboBox_currentTextChanged(const QString &arg1)
{
	configProtection();
}


void E2EReceiveRxMsg::on_countComboBox_currentTextChanged(const QString &arg1)
{
	configProtection();
}


void E2EReceiveRxMsg::on_protectionComboBox_currentTextChanged(const QString &arg1)
{
	configProtection();
	if(arg1 == "P11") {
		p11Widget.setVisible(true);
	} else {
		p11Widget.setVisible(false);
	}
}

