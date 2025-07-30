#include "e2eprotectsend.h"
#include "can.h"
#include "ui_e2eprotectsend.h"
#include <variant>

E2EProtectSend::E2EProtectSend(QWidget *parent) :
	QWidget(parent)
	, ui(new Ui::E2EProtectSend)
{
	ui->setupUi(this);
}

E2EProtectSend::E2EProtectSend(std::shared_ptr<dbcppp::INetwork> netPtr) :
	ui(new Ui::E2EProtectSend)
{
	ui->setupUi(this);
	this->netPtr = netPtr;

	for(int i = 0; i < netPtr->Messages_Size(); ++i) {
		const dbcppp::IMessage &msg = netPtr->Messages_Get(i);
		QString msgName = QString::fromStdString(msg.Name());
		ui->dbcListWidget->addItem(msgName);
	}

	ui->e2eTableWidget->clear();
	ui->e2eTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->mainHorizontalLayout->addWidget(&emptyTxMsg);

	workerTimerPtr = new QTimer(this);
	connect(workerTimerPtr, &QTimer::timeout, this, [this]() {
		runSM();
	});
	workerTimerPtr->start(1);
}

E2EProtectSend::~E2EProtectSend()
{
	delete ui;
	workerTimerPtr->stop();
	delete workerTimerPtr;

	for(const QString &key : e2eTxMsgMap.keys()) {
		delete e2eTxMsgMap[key];
	}
}

bool E2EProtectSend::isInE2E(QString msgName)
{
	return e2eTxMsgMap.contains(msgName);
}

void E2EProtectSend::on_addPushButton_clicked()
{
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
	int rowCount = ui->e2eTableWidget->rowCount();
	ui->e2eTableWidget->insertRow(rowCount);

	int msgIdx = -1;

	for(int i = 0; i < netPtr->Messages_Size(); ++i) {
		const dbcppp::IMessage &msg = netPtr->Messages_Get(i);
		QString name = QString::fromStdString(msg.Name());

		if(name == msgName) {
			msgIdx = i;
			break;
		}
	}

	E2EProtectTxMsg *txMsgPtr = new E2EProtectTxMsg(netPtr, msgName, msgIdx);

	e2eTxMsgMap.insert(msgName, txMsgPtr);

	ui->e2eTableWidget->setItem(rowCount, 0, &txMsgPtr->nameTableItem);
	ui->e2eTableWidget->setItem(rowCount, 1, &txMsgPtr->statusTableItem);

	connect(txMsgPtr, &E2EProtectTxMsg::sendCanMsg,
			this, &E2EProtectSend::onTxSendCanMsg);

	ui->mainHorizontalLayout->addWidget(txMsgPtr);
	txMsgPtr->setVisible(false);

	const dbcppp::IMessage &msg = netPtr->Messages_Get(e2eTxMsgMap[msgName]->getMsgIdx());

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
	int rowIdx = -1;
	for(int i = 0; i < ui->e2eTableWidget->rowCount(); ++i) {
		QTableWidgetItem *itemPtr = ui->e2eTableWidget->item(i, 0);
		if(msgName == itemPtr->text()) {
			rowIdx = i;
			break;
		}
	}

	ui->e2eTableWidget->takeItem(rowIdx, 0);
	ui->e2eTableWidget->takeItem(rowIdx, 1);
	ui->e2eTableWidget->removeRow(rowIdx);

	E2EProtectTxMsg *txMsgPtr = e2eTxMsgMap[msgName];

	if(txMsgPtr->isVisible()) {
		emptyTxMsg.setVisible(true);
	}

	e2eTxMsgMap.remove(msgName);
	delete txMsgPtr;

	if(e2eTxMsgMap.size() == 0) {
		emptyTxMsg.setVisible(true);
	}
}

void E2EProtectSend::on_removePushButton_clicked()
{
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

	emptyTxMsg.setVisible(false);
	for(const QString &key : e2eTxMsgMap.keys()) {
		e2eTxMsgMap[key]->setVisible(false);
	}
	QString msgName = ui->e2eTableWidget->item(row, 0)->text();
	e2eTxMsgMap[msgName]->setVisible(true);
}

void E2EProtectSend::closeEvent(QCloseEvent *event)
{
	(void)event;
	debug("Close event");
	emit closed(this);
}

void E2EProtectSend::onTxSendCanMsg(CanMsg canMsg)
{
	emit sendCanMsg(canMsg);
}

void E2EProtectSend::runSM(void)
{
	for(const QString &key : e2eTxMsgMap.keys()) {
		e2eTxMsgMap[key]->runSM();
	}
}
