#ifndef E2EPROTECTTXMSG_H
#define E2EPROTECTTXMSG_H

#include <QWidget>
#include <QTableWidgetItem>
#include <dbcppp/Network.h>
#include "can.h"
#include "e2e.hpp"
#include "e2eprotectsendp11.h"

class E2EProtectTxSig
{
public:
	E2EProtectTxSig(
		QString name,
		int value,
		int startBit,
		int len
	)
	{
		this->name.setText(name);
		this->value.setText(QString::number(value));
		this->startBit.setText(QString::number(startBit));
		this->len.setText(QString::number(len));

		this->name.setFlags(this->name.flags() & ~Qt::ItemIsEditable);
		this->startBit.setFlags(this->startBit.flags() & ~Qt::ItemIsEditable);
		this->len.setFlags(this->len.flags() & ~Qt::ItemIsEditable);
	}

	const int sigNameIdx = 0;
	const int sigValueIdx = 1;
	const int sigStartBitIdx = 2;
	const int sigLenIdx = 3;

	QTableWidgetItem name;
	QTableWidgetItem value;
	QTableWidgetItem startBit;
	QTableWidgetItem len;
};

namespace Ui {
class E2EProtectTxMsg;
}

class E2EProtectTxMsg : public QWidget
{
	Q_OBJECT

public:
	explicit E2EProtectTxMsg();
	explicit E2EProtectTxMsg(
		std::shared_ptr<dbcppp::INetwork> netPtr,
		QString name,
		int msgIdx
	);
	~E2EProtectTxMsg();
	QTableWidgetItem nameTableItem;
	QTableWidgetItem statusTableItem;
	int getMsgIdx(void);
	bool isCyclicSend(void) const;
	void runSM(void);

signals:
	void sendCanMsg(CanMsg canMsg);

private slots:
	void on_cyclicCheckBox_toggled(bool checked);
	void on_sendPushButton_clicked();

	void on_protectionComboBox_currentTextChanged(const QString &arg1);

private:
	Ui::E2EProtectTxMsg *ui;
	std::shared_ptr<dbcppp::INetwork> netPtr;
	bool isSending;
	int msgIdx;
	uint32_t id;
	double cycleTime;
	int dlc;
	QVector<E2EProtectTxSig *> sigsPtrVec;
	CanMsg canMsg;
	bool isThereReq;
	uint64_t timer;
	const uint64_t TIMER_IDLE = UINT64_MAX;
	E2EProtectSendP11 p11Widget;
	std::unique_ptr<E2E::P11Config> p11ConfigPtr;
	std::unique_ptr<E2E::P11> p11Ptr;
	QString protectProfileStr;

	/**
	 *  @brief Tries to find E2E configuration from message's attributes
	 *  It looks for below attributes
	 *  E2EDataId
	 *  E2EProfile
	 */
	void tryFindE2ECfg(void);
	/**
	 *  @brief Tries to find CRC and counter signal from message's signal's name
	 *  It looks for below keywords in signal names
	 *  count
	 *  crc
	 */
	void tryFindCrcAndCounter(void);
	/**
	 *  @brief Tries to find message cycle time from message's attributes
	 *  It looks for below attributes
	 *  GenMsgCycleTime
	 */
	void tryFindMsgCycle(void);
	void updateLastSentDataLabel(void);
	void addSignal(
		const std::string &nameRef,
		uint64_t startBit,
		uint64_t len
	);
	void createCanMsg(void);
	uint32_t getCounterStartBit(void);
	uint32_t getCrcStartBit(void);
	uint64_t getSignalValue(QString signalStr);
	void configProtection(void);
	uint32_t getStartBit(QString signalName);
};

#endif // E2EPROTECTTXMSG_H
