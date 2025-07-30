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
}


void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	qDebug() << item->text(column) << " feature triggered";

	if(item->text(column) != "E2E_Protect_Send") {
		// do nothing
	} else if(netPtr == nullptr) {
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
			&MainWindow::e2eProtectSendCanMsg
		);
		e2eProtectSendPtr->setVisible(true);
		qDebug() << "Opened E2E_Protect_Send";
		e2eProtectSendPtrVect.append(e2eProtectSendPtr);
	}
}

bool MainWindow::isAllDbcRelatedWinClosed(void) const
{
	bool ret = true;
	if(e2eProtectSendPtrVect.size() != 0) {
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

void MainWindow::e2eProtectSendCanMsg(CanMsg canMsg)
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
		} else if(canInt == CanInt::Std) {
			try {
				canPtr->disconnect();
				ui->connectionStatusLabel->setText("Offline");
			} catch (const std::runtime_error &e) {
				std::cout << e.what() << std::endl;
				ui->connectPushButton->setChecked(true);
			}
		}

		if(canPtr != nullptr) {
			canPtr.reset();
		}
	}
}

