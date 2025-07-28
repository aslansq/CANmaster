#include "peakstdcan.h"
#include <QDebug>

PeakStdCan::PeakStdCan(QObject *parent)
	: Can(parent)
	, pcanHandle(invalidPcanHandle)
{

}

PeakStdCan::~PeakStdCan()
{}

void PeakStdCan::connect(
	std::variant<CanStdConfig, CanFdConfig> config
)
{
	if(isConnected()) {
		qDebug() << "PEAK already connected";
		return;
	}

	CanStdConfig cfg = std::get<CanStdConfig>(config);
	TPCANStatus stsResult = PCAN_ERROR_OK;
	this->pcanBaud = getPcanBaud(cfg.baud);
	this->pcanHandle = getPeakHandle(cfg.channel);

	if(this->pcanHandle == this->invalidPcanHandle) {
		throw std::runtime_error("Invalid PCAN handle");
	}

	if(this->pcanBaud == this->invalidPcanBaud) {
		throw std::runtime_error("Invalid PCAN baudrate");
	}

	stsResult = CAN_Initialize(this->pcanHandle, this->pcanBaud);

	if (stsResult != PCAN_ERROR_OK) {
		qDebug() << "PEAK " << getStatusStr(stsResult);
		throw std::runtime_error("PEAK connection failed");
	} else {
		qDebug() << "PEAK connected";
		startRxThread();
		startTxThread();
		emit eventOccured(CanEvent::Connected);
	}
}

void PeakStdCan::disconnect(void)
{
	if(!isConnected()) {
		qDebug() << "PEAK not Connected to disconnect!";
	}
	stopRxThread();
	stopTxThread();
	CAN_Uninitialize(this->pcanHandle);
	this->pcanHandle = this->invalidPcanHandle;
	qDebug() << "PEAK disconnected";
	emit eventOccured(CanEvent::Disconnected);
}

void PeakStdCan::rx()
{
	TPCANMsg peakCanMsg;
	TPCANTimestamp peakTimestamp;
	TPCANStatus peakResult = PCAN_ERROR_OK;
	CanMsg canMsg;

	while(peakResult == PCAN_ERROR_OK) {
		peakResult = CAN_Read(this->pcanHandle, &peakCanMsg, &peakTimestamp);
		if(peakResult == PCAN_ERROR_OK) {
			peakStdMsgToCanMsg(peakCanMsg, peakTimestamp, canMsg);
			this->rxQueue.enqueue(canMsg);
			emit eventOccured(CanEvent::MessageReceived);
		}
		QThread::usleep(100);
	}
}

void PeakStdCan::tx()
{
	TPCANMsg peakCanMsg;
	CanMsg canMsg;
	bool isTxEmpty = this->txQueue.isEmpty();

	while(isTxEmpty == false) {
		canMsg = this->txQueue.dequeue();
		canMsgToPeakStdMsg(canMsg, peakCanMsg);
		CAN_Write(this->pcanHandle, &peakCanMsg);
		isTxEmpty = this->txQueue.isEmpty();
		QThread::usleep(100);
	}
}

TPCANBaudrate PeakStdCan::getPcanBaud(int baudrate)
{
	TPCANBaudrate peakBaud = this->invalidPcanBaud;
	QMap<int, TPCANBaudrate> peakBauds = {
		{1000000, PCAN_BAUD_1M  },
		{800000 , PCAN_BAUD_800K},
		{500000 , PCAN_BAUD_500K},
		{250000 , PCAN_BAUD_250K},
		{125000 , PCAN_BAUD_125K},
		{100000 , PCAN_BAUD_100K},
		{95000  , PCAN_BAUD_95K },
		{83000  , PCAN_BAUD_83K },
		{50000  , PCAN_BAUD_50K },
		{47000  , PCAN_BAUD_47K },
		{33000  , PCAN_BAUD_33K },
		{20000  , PCAN_BAUD_20K },
		{10000  , PCAN_BAUD_10K },
		{5000   , PCAN_BAUD_5K  }
	};

	if(peakBauds.contains(baudrate)) {
		peakBaud = peakBauds[baudrate];
	}

	return peakBaud;
}

void PeakStdCan::peakStdMsgToCanMsg(
	const TPCANMsg &peakMsgRef,
	TPCANTimestamp peakTimestamp,
	CanMsg &canMsgRef
)
{
	uint32_t idMask = 0x7FF;
	canMsgRef.isExtd = false;
	if ((peakMsgRef.MSGTYPE & PCAN_MESSAGE_EXTENDED) == PCAN_MESSAGE_EXTENDED) {
		idMask = 0x1FFFFFFF;
		canMsgRef.isExtd = true;
	}

	canMsgRef.id = peakMsgRef.ID & idMask;
	canMsgRef.dataLength = peakMsgRef.LEN;
	memcpy(canMsgRef.data, peakMsgRef.DATA, peakMsgRef.LEN);
	canMsgRef.timestamp = peakTimestamp.millis;
}

void PeakStdCan::canMsgToPeakStdMsg(
	const CanMsg &canMsgRef,
	TPCANMsg &peakMsgRef
)
{
	peakMsgRef.ID = canMsgRef.id;
	peakMsgRef.MSGTYPE = PCAN_MESSAGE_STANDARD;
	if(canMsgRef.isExtd) {
		peakMsgRef.MSGTYPE = PCAN_MESSAGE_EXTENDED;
	}
	peakMsgRef.LEN = canMsgRef.dataLength;
	memcpy(
		peakMsgRef.DATA,
		canMsgRef.data,
		canMsgRef.dataLength
	);
}
