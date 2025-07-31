#include "e2eprotecttxmsg.h"
#include "ui_e2eprotecttxmsg.h"

// needed for empty placeholder creation
E2EProtectTxMsg::E2EProtectTxMsg() :
	ui(new Ui::E2EProtectTxMsg)
	, nameTableItem("")
	, statusTableItem("!")
	, msgIdx(-1)
	, cycleTime(-1.0)
	, timer(TIMER_IDLE)
{
	ui->setupUi(this);
	this->netPtr = nullptr;
	id = 0;
}

// normally this constructor should be used
E2EProtectTxMsg::E2EProtectTxMsg(
		std::shared_ptr<dbcppp::INetwork> netPtr,
		QString name,
		int msgIdx
	) :
	ui(new Ui::E2EProtectTxMsg)
	, nameTableItem(name)
	, statusTableItem("!")
	, msgIdx(msgIdx)
	, cycleTime(-1.0)
	, dlc(-1)
	, timer(TIMER_IDLE) // Initialize timer to idle state
{
	ui->setupUi(this);
	this->netPtr = netPtr;
	ui->nameLabel->setText(name);

	// Get a reference to the message in the DBC network
	const dbcppp::IMessage &msgRef = netPtr->Messages_Get(msgIdx);

	// Initialize CAN message properties
	id = msgRef.Id();
	dlc = msgRef.MessageSize();
	canMsg.id = id;
	canMsg.isExtd = (id > 0x7FF);
	canMsg.dataLength = dlc;
	memset(canMsg.data, 0, sizeof(canMsg.data));
	canMsg.timestamp = 0;

	updateLastSentDataLabel();

	tryFindMsgCycle();

	ui->idLabel->setText("0x" + QString::number(id, 16).toUpper());
	ui->cycleTimeLabel->setText(QString::number(cycleTime));
	ui->dlcLabel->setText(QString::number(dlc));

	// Add protection widgets to the layout but keep it hidden initially
	// for them to visible, they needed to be selected from protectionComboBox
	ui->protectionConfigVerticalLayout->addWidget(&p11Widget);
	p11Widget.setVisible(false);

	// Add signals to the signal table and combo boxes
	for(const dbcppp::ISignal &signal : msgRef.Signals()) {
		QString sigName = QString::fromStdString(signal.Name());
		ui->crcComboBox->addItem(sigName);
		ui->countComboBox->addItem(sigName);
		addSignal(
			signal.Name(),
			signal.StartBit(),
			signal.BitSize()
		);
	}

	tryFindCrcAndCounter();
	tryFindE2ECfg();
}

E2EProtectTxMsg::~E2EProtectTxMsg()
{
	delete ui;
	for(int i = 0; i < sigsPtrVec.size(); ++i) {
		delete sigsPtrVec[i];
	}
}

void E2EProtectTxMsg::tryFindMsgCycle(void)
{
	// Find the message cycle time
	// by looking for the "GenMsgCycleTime" attribute in the message
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

// try to find E2E configuration from message's attributes
void E2EProtectTxMsg::tryFindE2ECfg(void)
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

// try to find CRC and counter signal from message's signal's name
void E2EProtectTxMsg::tryFindCrcAndCounter(void)
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

int E2EProtectTxMsg::getMsgIdx(void)
{
	return msgIdx;
}

void E2EProtectTxMsg::addSignal(
		const std::string &nameRef,
		uint64_t startBit,
		uint64_t len
)
{
	// add new row to the signal table
	int r = ui->sigTableWidget->rowCount();
	ui->sigTableWidget->insertRow(r);

	E2EProtectTxSig *txSigPtr = new E2EProtectTxSig(
		QString::fromStdString(nameRef),
		0,
		(int)startBit,
		(int)len
	);

	sigsPtrVec.append(txSigPtr);

	// add the signal table items to the signal table
	ui->sigTableWidget->setItem(
		r,
		txSigPtr->sigNameIdx,
		&txSigPtr->name
	);

	ui->sigTableWidget->setItem(
		r,
		txSigPtr->sigValueIdx,
		&txSigPtr->value
	);

	ui->sigTableWidget->setItem(
		r,
		txSigPtr->sigStartBitIdx,
		&txSigPtr->startBit
	);

	ui->sigTableWidget->setItem(
		r,
		txSigPtr->sigLenIdx,
		&txSigPtr->len
	);
}

bool E2EProtectTxMsg::isCyclicSend(void) const
{
	return ui->cyclicCheckBox->isChecked() && ui->sendPushButton->isChecked();
}

void E2EProtectTxMsg::updateLastSentDataLabel(void)
{
	QString dataStr;
	for (int i = 0; i < canMsg.dataLength; ++i) {
		dataStr += QString("%1 ").arg(canMsg.data[i], 2, 16, QChar('0')).toUpper();
	}
	dataStr = dataStr.trimmed();
	ui->lastSentDataLabel->setText(dataStr);
}

// Create the CAN message based on the current signal values
// and protection settings
void E2EProtectTxMsg::createCanMsg(void)
{
	// Reset the CAN message
	canMsg.id = id;
	canMsg.isExtd = (id > 0x7FF);
	canMsg.dataLength = dlc;
	memset(canMsg.data, 0, sizeof(canMsg.data));
	canMsg.timestamp = 0;

	// reads the signal values from the signal table
	// and sets them to the CAN message data
	for (int i = 0; i < sigsPtrVec.size(); ++i) {
		E2EProtectTxSig *sigPtr = sigsPtrVec[i];
		uint8_t startBit = sigPtr->startBit.text().toInt();
		uint8_t len = sigPtr->len.text().toInt();
		uint64_t value = sigPtr->value.text().toInt();

		// Set the signal value in the CAN message data
		for (uint8_t j = 0; j < len; ++j) {
			if (startBit + j < (canMsg.dataLength * 8)) {
				canMsg.data[(startBit+j) / 8] |= (value & 1) << ((startBit+j) % 8);
				value >>= 1;
			}
		}
	}

	// protect the message with P11 if choosen
	if(
		(ui->protectionComboBox->currentText() == "P11") &&
		(p11Ptr != nullptr)
	) {
		std::vector<uint8_t> frame;
		frame.clear();
		for(int i = 0; i < canMsg.dataLength; ++i) {
			frame.push_back(canMsg.data[i]);
		}
		std::vector<uint8_t> protectedFrame;
		protectedFrame.clear();
		p11Ptr->protect(frame, protectedFrame);
		for(int i = 0; i < protectedFrame.size(); ++i) {
			canMsg.data[i] = protectedFrame[i];
		}
	}

	updateLastSentDataLabel();
}

void E2EProtectTxMsg::on_cyclicCheckBox_toggled(bool checked)
{
	qDebug() << "E2EProtectTxMsg cyclicCheckBox " << checked;
	ui->sendPushButton->setCheckable(checked);
}



void E2EProtectTxMsg::on_sendPushButton_clicked()
{
	qDebug() << "E2EProtectTxMsg send push button clicked";
	bool isChecked = ui->sendPushButton->isChecked();
	// if actively sending data, disable every configuration
	ui->cyclicCheckBox->setDisabled(isChecked);
	ui->protectionComboBox->setDisabled(isChecked);
	ui->crcComboBox->setDisabled(isChecked);
	ui->countComboBox->setDisabled(isChecked);

	if(ui->protectionComboBox->currentText() == "P11") {
		p11Widget.setDisabled(isChecked);
	}
	// generate request
	isThereReq = true;
}

// get value of the signal from signal table
uint64_t E2EProtectTxMsg::getSignalValue(QString signalStr)
{
	uint64_t ret = 0;
	for(int i = 0; i < sigsPtrVec.size(); ++i) {
		E2EProtectTxSig *ptr = sigsPtrVec[i];
		if(ptr->name.text() == signalStr) {
			ret = ptr->value.text().toInt();
			break;
		}
	}
	return ret;
}

// configure protection
void E2EProtectTxMsg::configProtection(void)
{
	if(ui->protectionComboBox->currentText() == "P11") {
		p11Ptr.reset();
		p11ConfigPtr.reset();
		p11ConfigPtr = std::make_unique<E2E::P11Config>();
		p11ConfigPtr->dataId = p11Widget.getDataId();
		p11ConfigPtr->dataLen = dlc * 8;
		p11ConfigPtr->dataIdMode =
			(p11Widget.getDataIdMode() == E2EProtectSendP11::DataIdMode::BOTH) ?
			E2E::P11DataIdModes::BOTH :
			E2E::P11DataIdModes::DATA_ID_NIBBLE;
		p11ConfigPtr->counterOffset = getCounterStartBit();
		p11ConfigPtr->crcOffset = getCrcStartBit();
		p11ConfigPtr->dataIdNibbleOffset = 0;
		p11ConfigPtr->maxDeltaCounter = 0;

		uint8_t count = getSignalValue(ui->countComboBox->currentText());

		p11Ptr = std::make_unique<E2E::P11>(
			E2E::P11Functionality::PROTECT,
			count,
			*p11ConfigPtr
		);
	}
}

// run state machine
// it does not really have explicity have states
// but you can think of
// timer downcounting = WAITING CYCLE TIME TO ELAPSE
// timer idle = IDLE STATE
// timer 0 = SEND
void E2EProtectTxMsg::runSM(void)
{
	if(isThereReq) { // if there is req, elapse the timer right away
		timer = 0;
		isThereReq = false;
		configProtection();
	}

	if(timer == 0) { // timer elapsed so send
		createCanMsg();
		emit sendCanMsg(canMsg);
		if(isCyclicSend()) {
			timer = cycleTime;
		} else {
			timer = TIMER_IDLE;
		}
	}

	if(timer != TIMER_IDLE && timer > 0) {
		timer--;
	}

	if(timer != TIMER_IDLE) {
		statusTableItem.setText(">");
	} else {
		statusTableItem.setText("!");
	}
}

uint32_t E2EProtectTxMsg::getCounterStartBit(void)
{
	QString countSigName = ui->countComboBox->currentText();
	return getStartBit(countSigName);
}

uint32_t E2EProtectTxMsg::getCrcStartBit(void)
{
	QString crcSigName = ui->crcComboBox->currentText();
	return getStartBit(crcSigName);
}

uint32_t E2EProtectTxMsg::getStartBit(QString signalName)
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

void E2EProtectTxMsg::on_protectionComboBox_currentTextChanged(const QString &arg1)
{
	p11Widget.setVisible(false);

	if(arg1 == "P11") {
		p11Widget.setVisible(true);
	}
}

