#ifndef E2ERECEIVERXMSG_H
#define E2ERECEIVERXMSG_H

#include <QWidget>
#include <QTableWidgetItem>
#include <memory>
#include <dbcppp/Network.h>

#include "can.h"
#include "e2e.hpp"
#include "e2ereceivecheckp11.h"

namespace Ui {
class E2EReceiveRxMsg;
}

class E2EReceiveRxMsg : public QWidget
{
	Q_OBJECT

public:
	explicit E2EReceiveRxMsg(QWidget *parent = nullptr);
	explicit E2EReceiveRxMsg(
		std::shared_ptr<dbcppp::INetwork> netPtr,
		QString name,
		int msgIdx
	);
	~E2EReceiveRxMsg();
	QTableWidgetItem nameTableItem;
	QTableWidgetItem statusTableItem;
public slots:
	void onCanMsgReceived(const CanMsg &canMsgRef);

private slots:
	void on_crcComboBox_currentTextChanged(const QString &arg1);

	void on_countComboBox_currentTextChanged(const QString &arg1);

	void on_protectionComboBox_currentTextChanged(const QString &arg1);

private:
	Ui::E2EReceiveRxMsg *ui;
	std::shared_ptr<dbcppp::INetwork> netPtr;
	int msgIdx;
	uint32_t id;
	double cycleTime;
	int dlc;
	CanMsg canMsg;
	E2EReceiveCheckP11 p11Widget;
	std::unique_ptr<E2E::P11Config> p11ConfigPtr;
	std::unique_ptr<E2E::P11> p11Ptr;
	QString protectProfileStr;
	QMap<E2E::P11Status, QString> p11StatusMap;
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
	void updateLastReadDataLabel(void);
	uint32_t getCounterStartBit(void);
	uint32_t getCrcStartBit(void);
	void configProtection(void);
	uint32_t getStartBit(QString signalName);
};

#endif // E2ERECEIVERXMSG_H
