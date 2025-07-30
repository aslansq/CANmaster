#include "e2ereceivecheck.h"
#include "ui_e2ereceivecheck.h"

E2EReceiveCheck::E2EReceiveCheck(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::E2EReceiveCheck)
{
	ui->setupUi(this);
}

E2EReceiveCheck::E2EReceiveCheck(std::shared_ptr<dbcppp::INetwork> netPtr)
    : ui(new Ui::E2EReceiveCheck)
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
	ui->mainHorizontalLayout->addWidget(&emptyRxMsg);
}

E2EReceiveCheck::~E2EReceiveCheck()
{
	delete ui;
	for(const QString &key : e2eRxMsgMap.keys()) {
		delete e2eRxMsgMap[key];
	}
}

void E2EReceiveCheck::onCanMsgReceived(const CanMsg &canMsgRef)
{
	emit receivedCanMsg(canMsgRef);
}

bool E2EReceiveCheck::isInE2E(QString msgName)
{
	return e2eRxMsgMap.contains(msgName);
}

void E2EReceiveCheck::addToE2E(QString msgName)
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

	E2EReceiveRxMsg *rxMsgPtr = new E2EReceiveRxMsg(netPtr, msgName, msgIdx);

	e2eRxMsgMap.insert(msgName, rxMsgPtr);

	ui->e2eTableWidget->setItem(rowCount, 0, &rxMsgPtr->nameTableItem);
	ui->e2eTableWidget->setItem(rowCount, 1, &rxMsgPtr->statusTableItem);

	connect(this, &E2EReceiveCheck::receivedCanMsg,
		rxMsgPtr, &E2EReceiveRxMsg::onCanMsgReceived);

	ui->mainHorizontalLayout->addWidget(rxMsgPtr);
	rxMsgPtr->setVisible(false);

	const dbcppp::IMessage &msg = netPtr->Messages_Get(msgIdx);

	debug(msgName + " added to e2e list");
}

void E2EReceiveCheck::removeFromE2E(QString msgName)
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

	E2EReceiveRxMsg *rxMsgPtr = e2eRxMsgMap[msgName];

	if(rxMsgPtr->isVisible()) {
		emptyRxMsg.setVisible(true);
	}

	e2eRxMsgMap.remove(msgName);
	delete rxMsgPtr;

	if(e2eRxMsgMap.size() == 0) {
		emptyRxMsg.setVisible(true);
	}
}

void E2EReceiveCheck::closeEvent(QCloseEvent *event)
{
	emit closed(this);
}

void E2EReceiveCheck::debug(QString msg)
{
	qDebug() << "E2EReceiveCheck: " << msg;
}

void E2EReceiveCheck::on_addPushButton_clicked()
{
	QListWidgetItem *currentItemPtr = ui->dbcListWidget->currentItem();
	if(currentItemPtr == nullptr) {
		debug("dbc msg is not item highlighted!");
	} else {
		QString s = currentItemPtr->text();
		if(isInE2E(s)) {
			debug(s + " is already in e2e list!");
		} else {
			addToE2E(s);
		}
	}
}


void E2EReceiveCheck::on_e2eTableWidget_cellClicked(int row, int column)
{
	qDebug() << "E2EReceiveCheck: cell clicked" << row << " " << column;

	emptyRxMsg.setVisible(false);
	for(const QString &key : e2eRxMsgMap.keys()) {
		e2eRxMsgMap[key]->setVisible(false);
	}
	QString msgName = ui->e2eTableWidget->item(row, 0)->text();
	e2eRxMsgMap[msgName]->setVisible(true);
}

