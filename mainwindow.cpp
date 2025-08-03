#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "peakstdcan.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	checkButtonGroupPtr = new QButtonGroup(this);
	checkButtonGroupPtr->addButton(ui->stdCheckBox);
	checkButtonGroupPtr->addButton(ui->fdCheckBox);
	checkButtonGroupPtr->addButton(ui->replayCheckBox);
	checkButtonGroupPtr->setExclusive(true);

	ui->fdCheckBox->setVisible(false);
	ui->replayCheckBox->setVisible(false);

	canStdFormPtr = new CanStdForm(this);

	ui->mainVerticalLayout->addWidget(canStdFormPtr);
	ui->mainVerticalLayout->addSpacerItem(
		new QSpacerItem(
			20,
			40,
			QSizePolicy::Minimum,
			QSizePolicy::Expanding
		)
	);

	ui->stdCheckBox->setCheckState(Qt::Checked);
	ui->treeWidget->expandAll();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete checkButtonGroupPtr;
	delete canStdFormPtr;

	for(E2EProtectSend * e2eProtectSendPtr : e2eProtectSendPtrVect) {
		delete e2eProtectSendPtr;
	}

	for(E2EReceiveCheck * e2eReceiveCheckPtr : e2eReceiveCheckPtrVect) {
		delete e2eReceiveCheckPtr;
	}
}

void MainWindow::createE2EProtectSend(void)
{
	if(netPtr == nullptr) {
		QMessageBox::warning(
			nullptr,
			"Warning",
			"To open E2E_Protect_Send first load DBC"
		);
		qDebug() << "To open E2E_Protect_Send first load DBC";
	} else {
		auto e2eProtectSendPtr = new E2EProtectSend(netPtr);
		connect(
			e2eProtectSendPtr,
			&E2EProtectSend::closed,
			this,
			&MainWindow::e2eProtectSendClosed
		);
		connect(
			e2eProtectSendPtr,
			&E2EProtectSend::sendCanMsg,
			this,
			&MainWindow::onSendCanMsg
		);
		e2eProtectSendPtr->setVisible(true);
		qDebug() << "Opened E2E_Protect_Send";
		e2eProtectSendPtrVect.append(e2eProtectSendPtr);
	}
}

void MainWindow::createE2EReceiveCheck(void)
{
	if(netPtr == nullptr) {
		QMessageBox::warning(
			nullptr,
			"Warning",
			"To open E2E_Receive_Check first load DBC"
		);
		qDebug() << "To open E2E_Receive_Check first load DBC";
	} else {
		auto e2eReceiveCheckPtr = new E2EReceiveCheck(netPtr);
		connect(
			e2eReceiveCheckPtr,
			&E2EReceiveCheck::closed,
			this,
			&MainWindow::e2eReceiveCheckClosed
		);
		connect(
			this,
			&MainWindow::canMsgReceived,
			e2eReceiveCheckPtr,
			&E2EReceiveCheck::onCanMsgReceived
		);
		e2eReceiveCheckPtr->setVisible(true);
		qDebug() << "Opened E2E_Receive_Check";
		e2eReceiveCheckPtrVect.append(e2eReceiveCheckPtr);
	}
}

void MainWindow::createUdsFlasher(void)
{
	auto udsFlasherPtr = new UdsFlasher(this);
	connect(
		udsFlasherPtr,
		&UdsFlasher::closed,
		this,
		&MainWindow::udsFlasherClosed
	);
	connect(
		this,
		&MainWindow::canMsgReceived,
		udsFlasherPtr,
		&UdsFlasher::onCanMsgReceived
	);
	connect(
		udsFlasherPtr,
		&UdsFlasher::sendCanMsg,
		this,
		&MainWindow::onSendCanMsg
	);
	udsFlasherPtr->setVisible(true);
	qDebug() << "Opened UDS_Flasher";
	udsFlasherPtrVect.append(udsFlasherPtr);
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	QString featureName = item->text(column);
	qDebug() << featureName << " feature triggered";

	if(featureName == "E2E_Protect_Send") {
		createE2EProtectSend();
	} else if(featureName == "E2E_Receive_Check") {
		createE2EReceiveCheck();
	} else if(featureName == "UDS_Flasher") {
		createUdsFlasher();
	} else {
		qDebug() << "Unknown feature";
	}
}

bool MainWindow::isAllDbcRelatedWinClosed(void) const
{
	bool ret = true;
	if(e2eProtectSendPtrVect.size() != 0) {
		ret = false;
	}

	if(e2eReceiveCheckPtrVect.size() != 0) {
		ret = false;
	}

	return ret;
}

void MainWindow::on_dbcPushButton_clicked()
{
	qDebug() << "DBC push button clicked";

	if(!isAllDbcRelatedWinClosed()) {
		QMessageBox::warning(
			nullptr,
			"Warning",
			"To reload new DBC close all windows"
		);
		qDebug() << "To reload new DBC close all windows";
		return;
	}

	qDebug() << "Waiting user to select DBC file";

	QString fileName = QFileDialog::getOpenFileName(
		this,
		"Open CAN Database",
		QDir::homePath(),
		"DBC Files (*.dbc)"
	);

	if(fileName == "") {
		qDebug() << "DBC file empty, no parsing";
		return;
	}

	qDebug() << "DBC " << fileName << "selected";

	ui->dbcFileLabel->setText(fileName);

	std::filesystem::path filePath(fileName.toStdString());

	qDebug() << "Parsing DBC";

	auto net = dbcppp::INetwork::LoadNetworkFromFile(filePath);
	netPtr = std::move(net[""]);
}

void MainWindow::e2eProtectSendClosed(E2EProtectSend *ptr)
{
	e2eProtectSendPtrVect.removeOne(ptr);
	delete ptr;
}

void MainWindow::e2eReceiveCheckClosed(E2EReceiveCheck *ptr)
{
	e2eReceiveCheckPtrVect.removeOne(ptr);
	delete ptr;
}

void MainWindow::udsFlasherClosed(UdsFlasher *ptr)
{
	udsFlasherPtrVect.removeOne(ptr);
	delete ptr;
}

void MainWindow::onSendCanMsg(const CanMsg &canMsg)
{
	if(canPtr == nullptr) {
		qDebug() << "CAN interface not connected, cannot send message";
		return;
	}

	try {
		canPtr->txQueue.enqueue(canMsg);
	} catch (const std::exception &e) {
		qDebug() << "Error sending CAN message: " << e.what();
	}
}

MainWindow::CanInt MainWindow::getCanInt(void) const
{
	CanInt canInt;

	if(ui->stdCheckBox->isChecked()) {
		canInt = CanInt::Std;
	} else if(ui->fdCheckBox->isChecked()) {
		canInt = CanInt::Fd;
	} else if(ui->replayCheckBox->isChecked()) {
		canInt = CanInt::Replay;
	} else {
		throw std::runtime_error("Invalid can interface");
	}

	return canInt;
}

void MainWindow::canStdConnect(void)
{
	CanStdConfig canStdConfig;
	canStdConfig.interface = canStdFormPtr->getInterface();
	canStdConfig.baud = canStdFormPtr->getBaud();
	canStdConfig.channel = canStdFormPtr->getChannel();
	std::shared_ptr<PeakStdCan> peakStdCanPtr = std::make_shared<PeakStdCan>();
	try {
		peakStdCanPtr->connect(canStdConfig);
		canPtr = std::move(peakStdCanPtr);
		connect(
			canPtr.get(),
			&Can::eventOccured,
			this,
			&MainWindow::onCanEventOccured
		);
		ui->connectionStatusLabel->setText("Online");
	} catch (const std::runtime_error &e) {
		std::cout << e.what() << std::endl;
		ui->connectPushButton->setChecked(false);
	}
}

void MainWindow::on_connectPushButton_clicked(bool checked)
{
	CanInt canInt = getCanInt();

	if(checked) {
		if(canInt == CanInt::Std) {
			canStdConnect();
		}
	} else {
		if(canPtr == nullptr) {
			// there is nothing to do
		} else {
			try {
				canPtr->disconnect();
				ui->connectionStatusLabel->setText("Offline");
				disconnect(
					canPtr.get(),
					&Can::eventOccured,
					this,
					&MainWindow::onCanEventOccured
				);
			} catch (const std::runtime_error &e) {
				std::cout << e.what() << std::endl;
				ui->connectPushButton->setChecked(true);
			}
		}

		if(canPtr != nullptr) {
			canPtr.reset();
		}
	}

	canStdFormPtr->setDisabled(checked);
}

void MainWindow::onCanEventOccured(CanEvent event)
{
	if(event == CanEvent::MessageReceived) {
		CanMsg canMsg;
		try {
			canMsg = canPtr->rxQueue.dequeue();
			emit canMsgReceived(canMsg);
		} catch (const std::exception &e) {
			qDebug() << "Error receiving CAN message: " << e.what();
		}
	}
}
