/**
 * @file e2eprotecttxmsg.h
 * @brief This file defines the E2EProtectTxMsg class, which is used
 *        for managing the transmission of E2E protected messages.
 * @details The E2EProtectTxMsg class provides methods to configure and send E2E protected messages
 *          in a CAN network.
 */
#ifndef E2EPROTECTTXMSG_H
#define E2EPROTECTTXMSG_H

#include <QWidget>
#include <QTableWidgetItem>
#include <dbcppp/Network.h>

#include "can.h"
#include "e2e.hpp"
#include "e2eprotectsendp11.h"

/// @todo re-write this class to use value table items. right now it only uses value text
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

		/** @brief make the name, startBit, and len items non-editable
		 *  this is to prevent the user from accidentally changing these values
		 */
		this->name.setFlags(this->name.flags() & ~Qt::ItemIsEditable);
		this->startBit.setFlags(this->startBit.flags() & ~Qt::ItemIsEditable);
		this->len.setFlags(this->len.flags() & ~Qt::ItemIsEditable);
	}

	/** @brief Indexes for the signal name, value, start bit, and length in the table widget
	 *  It should match the order of the columns in the table widget
	 */
	const int sigNameIdx = 0;
	const int sigValueIdx = 1;
	const int sigStartBitIdx = 2;
	const int sigLenIdx = 3;

	/// @brief Table widget items for signal table
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
	/**
	 * @brief Default constructor for E2EProtectTxMsg.
	 *        This constructor is used when creating an empty instance.
	 */
	explicit E2EProtectTxMsg();
	/**
	 * @brief Constructor for E2EProtectTxMsg.
	 *        This constructor initializes the message with a DBC network and message index.
	 * @param netPtr A shared pointer to the DBC network.
	 * @param name The name of the message.
	 * @param msgIdx The index of the message in the DBC network.
	 */
	explicit E2EProtectTxMsg(
		std::shared_ptr<dbcppp::INetwork> netPtr,
		QString name,
		int msgIdx
	);
	~E2EProtectTxMsg();
	/// @brief Table widget items for the parent E2EProtectSend class
	QTableWidgetItem nameTableItem;
	QTableWidgetItem statusTableItem;
	int getMsgIdx(void);
	/**
	 * @brief Checks if the message is set to be sent cyclically.
	 * @return True if the message is cyclic, false otherwise.
	 */
	bool isCyclicSend(void) const;
	/**
	 * @brief Called from E2EProtectSend periodically to run the state machine.
	 */
	void runSM(void);

signals:
	/**
	 * @brief Signal emitted when a CAN message is sent.
	 * @param canMsg The CAN message that was sent.
	 */
	void sendCanMsg(CanMsg canMsg);

private slots:
	void on_cyclicCheckBox_toggled(bool checked);
	void on_sendPushButton_clicked();

	void on_protectionComboBox_currentTextChanged(const QString &arg1);

private:
	Ui::E2EProtectTxMsg *ui;
	/// @brief Pointer to the DBC network.
	std::shared_ptr<dbcppp::INetwork> netPtr;
	/// @brief Index of the message in the DBC network.
	int msgIdx;
	/// @brief CAN message ID.
	uint32_t id;
	/// @brief Cycle time of the message.
	double cycleTime;
	/// @brief Data Length Code (DLC) of the CAN message.
	int dlc;
	/// @brief Signals associated with the signal table for the message.
	QVector<E2EProtectTxSig *> sigsPtrVec;
	CanMsg canMsg;
	/// @brief Send button click generates this request.
	bool isThereReq;
	/// @brief Timer for the state machine.
	uint64_t timer;
	/// @brief Timer value indicating that the timer is idle.
	const uint64_t TIMER_IDLE = UINT64_MAX;
	/// @brief Protection P11 widget for E2E protection.
	/// @details This widget is used to configure P11 protection settings.
	E2EProtectSendP11 p11Widget;
	/// @brief P11 configuration pointer of the algorithm itself.
	std::unique_ptr<E2E::P11Config> p11ConfigPtr;
	/// @brief P11 algorithm pointer for E2E protection.
	std::unique_ptr<E2E::P11> p11Ptr;

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
	 *  @brief Updates the last sent data label in the UI
	 */
	void updateLastSentDataLabel(void);
	/**
	 *  @brief Adds a signal to the signal table and to the sigsPtrVec vector.
	 *  @param nameRef The name of the signal
	 *  @param startBit The start bit of the signal
	 *  @param len The length of the signal
	 */
	void addSignal(
		const std::string &nameRef,
		uint64_t startBit,
		uint64_t len
	);
	/// @brief creates a CAN message based on the current protection profile and signals
	void createCanMsg(void);
	uint32_t getCounterStartBit(void);
	uint32_t getCrcStartBit(void);
	/**
	 * @brief Gets the value of a signal by its name.
	 * @param signalStr The name of the signal.
	 * @return The value of the signal as an integer.
	 */
	uint64_t getSignalValue(QString signalStr);
	/// @brief Configures the protection settings for the message by looking at the UI elements.
	void configProtection(void);
	/// @brief Gets the start bit of a signal by its name from DBC network.
	/// @param signalName The name of the signal.
	/// @return The start bit of the signal.
	uint32_t getStartBit(QString signalName);
};

#endif // E2EPROTECTTXMSG_H
