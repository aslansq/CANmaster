#include "e2eprotectsend.h"
#include "can.h"
#include "ui_e2eprotectsend.h"
#include <variant>
#include <string>

E2EProtectSend::E2EProtectSend(std::shared_ptr<dbcppp::INetwork> netPtr) :
	ui(new Ui::E2EProtectSend)
{
	ui->setupUi(this);
	this->netPtr = netPtr;

	// find all messages in DBC and add them to the list widget
	for(int i = 0; i < netPtr->Messages_Size(); ++i) {
		const dbcppp::IMessage &msg = netPtr->Messages_Get(i);
		QString msgName = QString::fromStdString(msg.Name());
		ui->dbcListWidget->addItem(msgName);
	}

	// set up the E2E table widget
	ui->e2eTableWidget->clear();
	// disable editing of the table
	ui->e2eTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// add placeholder for empty E2E message
	ui->mainHorizontalLayout->addWidget(&emptyTxMsg);

	// create 1 ms timer for running the state machine
	// this will call runSM() every 1 ms
	workerTimerPtr = new QTimer(this);
	connect(workerTimerPtr, &QTimer::timeout, this, [this]() {
		runSM();
	});
	workerTimerPtr->start(1);
}

E2EProtectSend::~E2EProtectSend()
{
	delete ui;
	// stop the timer and delete it
	workerTimerPtr->stop();
	delete workerTimerPtr;
	// delete all E2EProtectTxMsg instances
	for(const QString &key : e2eTxMsgMap.keys()) {
		delete e2eTxMsgMap[key];
	}
}

bool E2EProtectSend::isInE2E(QString msgName)
{
	// Check if the message name is already in the E2E list
	return e2eTxMsgMap.contains(msgName);
}

void E2EProtectSend::on_addPushButton_clicked()
{
	debug("on_addPushButton_clicked");
	// Get the currently highlighted item in the DBC list widget
	// and add it to the E2E list if it's not already there
	QListWidgetItem *currentItemPtr = ui->dbcListWidget->currentItem();
	if(currentItemPtr == nullptr) {
		debug("dbc msg is not item highlighted!");
	} else {
		QString s = currentItemPtr->text();
		if(isInE2E(s)) {
			debug(s + " is in already in e2e list!");
		} else {
			addToE2E(s);
		}
	}
}

void E2EProtectSend::addToE2E(QString msgName)
{
	debug("addToE2E: " + msgName);
	// creating new row
	int rowCount = ui->e2eTableWidget->rowCount();
	ui->e2eTableWidget->insertRow(rowCount);

	// trying to find the message in the DBC network
	int msgIdx = -1;
	for(int i = 0; i < netPtr->Messages_Size(); ++i) {
		const dbcppp::IMessage &msg = netPtr->Messages_Get(i);
		QString name = QString::fromStdString(msg.Name());

		if(name == msgName) {
			msgIdx = i;
			break;
		}
	}

	// this should never happen, but just in case
	if(msgIdx < 0) {
		throw std::runtime_error("Message " + msgName.toStdString() + " not found in DBC network!");
	}
	
	// create a new E2EProtectTxMsg instance and add it to the e2eTxMsgMap
	E2EProtectTxMsg *txMsgPtr = new E2EProtectTxMsg(netPtr, msgName, msgIdx);

	e2eTxMsgMap.insert(msgName, txMsgPtr);

	ui->e2eTableWidget->setItem(rowCount, 0, &txMsgPtr->nameTableItem);
	ui->e2eTableWidget->setItem(rowCount, 1, &txMsgPtr->statusTableItem);

	// E2EProtectTxMsg instance sends CAN messages
	// when the user clicks the send button, so we connect the signal
	// from E2EProtectTxMsg to the onTxSendCanMsg slot in E2EProtectSend
	// this way we can handle the sending of CAN messages in one place
	connect(txMsgPtr, &E2EProtectTxMsg::sendCanMsg,
			this, &E2EProtectSend::onTxSendCanMsg);

	// add the E2EProtectTxMsg instance to the main layout
	// initially it is not visible
	// to make it visible, the user has to click on the row in the E2E table widget
	ui->mainHorizontalLayout->addWidget(txMsgPtr);
	txMsgPtr->setVisible(false);

	// just for debugging, print the message attributes
	const dbcppp::IMessage &msg = netPtr->Messages_Get(msgIdx);

	for(int i = 0; i < msg.AttributeValues_Size(); ++i) {
		const dbcppp::IAttribute& attr = msg.AttributeValues_Get(i);
		dbcppp::IAttribute::value_t value = attr.Value();
		std::cout << attr.Name() << " ";
		if(std::holds_alternative<int64_t>(value)) {
			std::cout << (int)std::get<int64_t>(value);
		} else if(std::holds_alternative<std::string>(value)) {
			std::cout << std::get<std::string>(value);
		} else {
			std::cout << std::get<double>(value);
		}
		std::cout << std::endl;
	}

	debug(msgName + " added to e2e list");
}

void E2EProtectSend::removeFromE2E(QString msgName)
{
	debug("removeFromE2E: " + msgName);
	// this should never happen, but just in case
	if(!isInE2E(msgName)) {
		throw std::runtime_error("Message " + msgName.toStdString() + " not found in E2E list to remove!");
	}
	// try to find the row index of the message in the E2E table widget
	int rowIdx = -1;
	for(int i = 0; i < ui->e2eTableWidget->rowCount(); ++i) {
		QTableWidgetItem *itemPtr = ui->e2eTableWidget->item(i, 0);
		if(msgName == itemPtr->text()) {
			rowIdx = i;
			break;
		}
	}

	// remove the row from the E2E table widget
	ui->e2eTableWidget->takeItem(rowIdx, 0);
	ui->e2eTableWidget->takeItem(rowIdx, 1);
	ui->e2eTableWidget->removeRow(rowIdx);

	// remove the E2EProtectTxMsg instance from the e2eTxMsgMap
	// and delete it

	// this should never happen, but just in case
	if(!e2eTxMsgMap.contains(msgName)) {
		throw std::runtime_error("Message " + msgName.toStdString() + " not found in E2E list to remove!");
	}

	E2EProtectTxMsg *txMsgPtr = e2eTxMsgMap[msgName];

	if(txMsgPtr->isVisible()) {
		emptyTxMsg.setVisible(true);
	}

	e2eTxMsgMap.remove(msgName);
	delete txMsgPtr;

	// if there is no message left in the E2E list to show details,
	// we show the emptyTxMsg placeholder
	if(e2eTxMsgMap.size() == 0) {
		emptyTxMsg.setVisible(true);
	}
}

void E2EProtectSend::on_removePushButton_clicked()
{
	debug("on_removePushButton_clicked");
	// Get the currently highlighted row in the E2E table widget
	// and remove the corresponding message from the E2E list
	int rowIdx = ui->e2eTableWidget->currentRow();

	if(rowIdx < 0 || rowIdx >= e2eTxMsgMap.size()) {
		// do nothing
	} else {
		QTableWidgetItem *itemPtr = ui->e2eTableWidget->item(rowIdx, 0);
		QString msgName = itemPtr->text();
		removeFromE2E(msgName);
	}
}

void E2EProtectSend::debug(const QString& s)
{
	qDebug() << "E2EProtectSend: " << s;
}

void E2EProtectSend::on_e2eTableWidget_cellClicked(int row, int column)
{
	qDebug() << "E2EProtectSend: cell clicked" << row << " " << column;
	// disable the show of every E2EProtectTxMsg instance
	emptyTxMsg.setVisible(false);
	for(const QString &key : e2eTxMsgMap.keys()) {
		e2eTxMsgMap[key]->setVisible(false);
	}
	// get the message name from the clicked row and show the corresponding E2EProtectTxMsg instance
	QString msgName = ui->e2eTableWidget->item(row, 0)->text();
	e2eTxMsgMap[msgName]->setVisible(true);
}

void E2EProtectSend::closeEvent(QCloseEvent *event)
{
	// let parent class handle the close event
	(void)event;
	debug("Close event");
	emit closed(this);
}

void E2EProtectSend::onTxSendCanMsg(CanMsg canMsg)
{
	// let parent class handle the sending of CAN messages
	emit sendCanMsg(canMsg);
}

void E2EProtectSend::runSM(void)
{
	// run the state machine for each E2EProtectTxMsg instance
	for(const QString &key : e2eTxMsgMap.keys()) {
		e2eTxMsgMap[key]->runSM();
	}
}
