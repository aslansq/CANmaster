#include "udsflasher.h"
#include "ui_udsflasher.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <dlfcn.h>

#include "seednkey.h"

UdsFlasher::UdsFlasher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UdsFlasher)
    , flashThreadPtr(nullptr)
    , flashWorkerPtr(nullptr)
    , isFlashing(false)
{
	ui->setupUi(this);
	setHexValidators();

	settings = std::make_unique<QSettings>("CanMaster", "UdsFlasher");

	ui->binFileLineEdit->setText(settings->value("binFilePath", "").toString());
	ui->addrLineEdit->setText(settings->value("address", "").toString());
	ui->reqIdLineEdit->setText(settings->value("reqId", "").toString());
	ui->respIdLineEdit->setText(settings->value("respId", "").toString());
	ui->seednkeyFileLineEdit->setText(settings->value("seednkeyFilePath", "").toString());
	ui->timeoutSpinBox->setValue(settings->value("timeout", 2000).toInt());
	ui->securityLevelSpinBox->setValue(settings->value("securityLevel", 1).toInt());

	rxCanMsgQueuePtr = new ThreadSafeQueue<CanMsg>();
}

UdsFlasher::~UdsFlasher()
{
	delete ui;
	delete rxCanMsgQueuePtr;
}

void UdsFlasher::setHexValidators()
{
	// Assuming addrLineEdit is a QLineEdit in the UI
	QRegularExpression hexValidator("[0-9A-Fa-f]{1,8}");
	ui->addrLineEdit->setValidator(new QRegularExpressionValidator(hexValidator, this));
	ui->reqIdLineEdit->setValidator(new QRegularExpressionValidator(hexValidator, this));
	ui->respIdLineEdit->setValidator(new QRegularExpressionValidator(hexValidator, this));
}

uint32_t UdsFlasher::getAddr(void) const
{
	bool ok;
	uint32_t addr = ui->addrLineEdit->text().toUInt(&ok, 16);
	if (!ok) {
		throw std::invalid_argument("Invalid address format");
	}
	return addr;
}

uint32_t UdsFlasher::getReqId(void) const
{
	bool ok;
	uint32_t reqId = ui->reqIdLineEdit->text().toUInt(&ok, 16);
	if (!ok) {
		throw std::invalid_argument("Invalid request ID format");
	}
	return reqId;
}

uint32_t UdsFlasher::getRespId(void) const
{
	bool ok;
	uint32_t respId = ui->respIdLineEdit->text().toUInt(&ok, 16);
	if (!ok) {
		throw std::invalid_argument("Invalid response ID format");
	}
	return respId;
}

void UdsFlasher::closeEvent(QCloseEvent *event)
{
	// let parent class handle the close event
	(void)event;
	emit closed(this);
}

bool UdsFlasher::isFlashPrecondOk(void) const
{
	// Check if the preconditions for flashing are met
	if (ui->binFileLineEdit->text().isEmpty()) {
		return false; // No binary file selected
	}
	if (ui->addrLineEdit->text().isEmpty()) {
		return false; // No address specified
	}
	if (ui->reqIdLineEdit->text().isEmpty()) {
		return false; // No request ID specified
	}
	if (ui->respIdLineEdit->text().isEmpty()) {
		return false; // No response ID specified
	}
	if (ui->seednkeyFileLineEdit->text().isEmpty()) {
		return false; // No seed and key file specified
	}
	return true; // Precondition checks passed
}

void UdsFlasher::setConfigUiDisabled(bool disabled)
{
	ui->binFilePushButton->setDisabled(disabled);
	ui->binFileLineEdit->setDisabled(disabled);
	ui->addrLineEdit->setDisabled(disabled);
	ui->timeoutSpinBox->setDisabled(disabled);
	ui->reqIdLineEdit->setDisabled(disabled);
	ui->respIdLineEdit->setDisabled(disabled);
	ui->seednkeyFilePushButton->setDisabled(disabled);
	ui->seednkeyFileLineEdit->setDisabled(disabled);
	ui->securityLevelSpinBox->setDisabled(disabled);
}

void UdsFlasher::onWorkerSendCanMsg(const CanMsg &canMsg)
{
	emit sendCanMsg(canMsg);
}

void UdsFlasher::onWorkerSendLog(const QString &s)
{
	ui->logTextBrowser->append(s);
}

void UdsFlasher::onCanMsgReceived(const CanMsg &canMsgRef)
{
	if(isFlashing && canMsgRef.id == getRespId()) {
		rxCanMsgQueuePtr->enqueue(canMsgRef);
	}
}

void UdsFlasher::onFlashWorkerFinished(void)
{
	if (flashThreadPtr != nullptr) {
		flashThreadPtr->quit();
		flashThreadPtr->wait();
		delete flashThreadPtr;
	}
	isFlashing = false;
	ui->flashPushButton->setChecked(false);
	ui->flashPushButton->setDisabled(false);
	setConfigUiDisabled(false);
}

void UdsFlasher::on_flashPushButton_clicked()
{
	bool isChecked = ui->flashPushButton->isChecked();

	setConfigUiDisabled(isChecked);

	if (isChecked) {
		if( !isFlashPrecondOk()) {
			ui->statusbar->showMessage(tr("Please ensure all preconditions are met before flashing."), 3000);
			ui->flashPushButton->setChecked(false);
			setConfigUiDisabled(false);
		} else if(!isFlashing){
			ui->logTextBrowser->setPlainText("");
			flashThreadPtr = new QThread();
			flashThreadPtr->setObjectName("UdsFlashThread");
			flashWorkerPtr = new UdsFlashWorker(
				nullptr,
				rxCanMsgQueuePtr,
				getReqId(),
				getRespId(),
				ui->binFileLineEdit->text(),
				ui->seednkeyFileLineEdit->text(),
				ui->securityLevelSpinBox->value(),
				getAddr(),
				ui->timeoutSpinBox->value()
			);
			flashWorkerPtr->moveToThread(flashThreadPtr);
			connect(
				flashThreadPtr,
				&QThread::started,
				flashWorkerPtr,
				&UdsFlashWorker::run
			);
			connect(
				flashWorkerPtr,
				&UdsFlashWorker::finished,
				this,
				&UdsFlasher::onFlashWorkerFinished
			);
			connect(
				flashWorkerPtr,
				&UdsFlashWorker::sendCanMsg,
				this,
				&UdsFlasher::onWorkerSendCanMsg
			);
			connect(
				flashWorkerPtr,
				&UdsFlashWorker::sendLog,
				this,
				&UdsFlasher::onWorkerSendLog
			);
			flashThreadPtr->start();
			isFlashing = true;
			ui->flashPushButton->setDisabled(true);
		}
	}
}

void UdsFlasher::on_binFilePushButton_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Binary File"), "", tr("Binary Files (*.bin)"));
	if (!fileName.isEmpty()) {
		ui->binFileLineEdit->setText(fileName);
	}
	settings->setValue("binFilePath", ui->binFileLineEdit->text());
}


void UdsFlasher::on_seednkeyFilePushButton_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Seed and Key File"), "", tr("Seed and Key Files (*.so)"));
	if (!fileName.isEmpty()) {
		ui->seednkeyFileLineEdit->setText(fileName);
	}
	settings->setValue("seednkeyFilePath", ui->seednkeyFileLineEdit->text());
}


void UdsFlasher::on_timeoutSpinBox_valueChanged(int arg1)
{
	(void)arg1;
	settings->setValue("timeout", ui->timeoutSpinBox->value());
}


void UdsFlasher::on_addrLineEdit_textChanged(const QString &arg1)
{
	(void)arg1;
	settings->setValue("address", ui->addrLineEdit->text());
}


void UdsFlasher::on_securityLevelSpinBox_valueChanged(int arg1)
{
	(void)arg1;
	settings->setValue("securityLevel", ui->securityLevelSpinBox->value());
}


void UdsFlasher::on_respIdLineEdit_textChanged(const QString &arg1)
{
	(void)arg1;
	settings->setValue("respId", ui->respIdLineEdit->text());
}


void UdsFlasher::on_reqIdLineEdit_textChanged(const QString &arg1)
{
	(void)arg1;
	settings->setValue("reqId", ui->reqIdLineEdit->text());
}

