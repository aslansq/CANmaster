/**
 * @file e2ereceiverxmsg.h
 * @brief This file defines the E2EReceiveRxMsg class, which is used
 *        for managing the E2E receive check configuration gui elements.
 * @details The E2EReceiveRxMsg class provides methods to set and get
 *          various E2E related configurations for a specific message.
 */

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
	/// needed constructor for empty placeholder
	explicit E2EReceiveRxMsg(QWidget *parent = nullptr);
	/**
	 * @brief Constructor that initializes the message with a network pointer, name, and message index
	 */
	explicit E2EReceiveRxMsg(
		std::shared_ptr<dbcppp::INetwork> netPtr,
		QString name,
		int msgIdx
	);
	~E2EReceiveRxMsg();
	/// @brief Table item for the message name. This is used to display the message name in a table in E2EReceiveCheck.
	QTableWidgetItem nameTableItem;
	/// @brief Table item for the message status. This is used to display the message status in a table in E2EReceiveCheck.
	QTableWidgetItem statusTableItem;
public slots:
	void onCanMsgReceived(const CanMsg &canMsgRef);

private slots:
	void on_crcComboBox_currentTextChanged(const QString &arg1);

	void on_countComboBox_currentTextChanged(const QString &arg1);

	void on_protectionComboBox_currentTextChanged(const QString &arg1);

	void onP11ConfigChanged(void);

private:
	Ui::E2EReceiveRxMsg *ui;
	/// @brief Pointer to the DBC network interface
	std::shared_ptr<dbcppp::INetwork> netPtr;
	/// @brief Message index in the DBC network
	int msgIdx;
	/// @brief Message ID
	uint32_t id;
	/// @brief Message cycle time in milliseconds
	double cycleTime;
	/// @brief Message data length code
	int dlc;
	CanMsg canMsg;
	/// @brief Widget for configuring P11 protection
	E2EReceiveCheckP11 p11Widget;
	/// @brief Algorithim configuration pointer for P11
	std::unique_ptr<E2E::P11Config> p11ConfigPtr;
	/// @brief Algorithim pointer for P11
	std::unique_ptr<E2E::P11> p11Ptr;
	/// @brief Protection profile string, e.g., "P11", "NONE
	QString protectProfileStr;
	/// @brief Map to convert P11 status enum to string for display
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
	/**
	 * @brief Updates the last read data label with the current CAN message data
	 * @details Converts the CAN message data to a hexadecimal string and updates the label
	 */
	void updateLastReadDataLabel(void);
	uint32_t getCounterStartBit(void);
	uint32_t getCrcStartBit(void);
	/// @brief Configures the protection settings based on the selected profile
	void configProtection(void);
	/// @brief Gets the start bit for a specific signal from DBC network
	/// @param signalName The name of the signal
	/// @return The start bit position of the signal
	uint32_t getStartBit(QString signalName);
};

#endif // E2ERECEIVERXMSG_H
