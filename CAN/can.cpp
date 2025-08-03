#include "can.h"

Can::Can(QObject *parent) :
	QObject(parent),
	rxThread(nullptr),
	txThread(nullptr)
{
	qRegisterMetaType<CanEvent>("CanEvent");
}

bool Can::isConnected(void) const
{
	return this->rxThread != nullptr;
}

void Can::stopRxThread(void)
{
	if(this->rxThread != nullptr) {
		this->rxThread->exit();
		this->rxThread = nullptr;
	}
}

void Can::startRxThread(void)
{
	this->rxThread = QThread::create([this]() {
		while(isConnected()) {
			rx();
		}
	});
	this->rxThread->start();
}

void Can::stopTxThread(void)
{
	if(this->txThread != nullptr) {
		this->txThread->exit();
		this->txThread = nullptr;
	}
}

void Can::startTxThread(void)
{
	this->txThread = QThread::create([this]() {
		while(isConnected()) {
			tx();
		}
	});
	this->txThread->start();
}
