/**
 * @file e2eprotectsend.h
 * @brief Main implementation file for E2E protection send functionality.
 * @details This file defines the E2EProtectSend class, which manages the sending of
 *          E2E protected messages in a CAN network.
 */

#ifndef E2EPROTECTSEND_H
#define E2EPROTECTSEND_H

#include <QWidget>
#include <QMap>
#include <QTimer>
#include <memory>
#include <dbcppp/Network.h>

#include "e2eprotecttxmsg.h"
#include "can.h"

namespace Ui {
class E2EProtectSend;
}

class E2EProtectSend : public QWidget
{
	Q_OBJECT

public:
	/**
	 * @brief reads the DBC network and initializes the E2EProtectSend widget with CAN messages.
	 * @param netPtr A shared pointer to the DBC network.
	 */
	E2EProtectSend(std::shared_ptr<dbcppp::INetwork> netPtr);
	~E2EProtectSend();
signals:
	/**
	 * @brief Signal emitted when user presses the close button.
	 * @param ptr Pointer to the this E2EProtectSend instance.
	 */
	void closed(E2EProtectSend *ptr);
	/**
	 * @brief Signal emitted when a CAN message is sent.
	 * @param canMsg The CAN message that was sent.
	 */
	void sendCanMsg(const CanMsg &canMsg);

private slots:
	/**
	 * @brief Checks higlighted DBC message and adds it to E2E list.
	 */
	void on_addPushButton_clicked();
	/**
	 * @brief Checks highlighted DBC message in E2E list and removes it.
	 */
	void on_removePushButton_clicked();
	/**
	 * @brief Shows the E2E message details when a row in the E2E table is clicked.
	 */
	void on_e2eTableWidget_cellClicked(int row, int column);
	/**
	 * @brief Handles the sending of CAN messages.
	 * @details This slot is connected to the sendCanMsg signal from E2EProtectTxMsg.
	 * @param canMsg The CAN message to be sent.
	 */
	void onTxSendCanMsg(CanMsg canMsg);

private:
	Ui::E2EProtectSend *ui;
	/// @brief Pointer to the DBC network.
	std::shared_ptr<dbcppp::INetwork> netPtr;
	/// @brief Map of E2E protected messages indexed by their names.
	/// @details This map holds instances of E2EProtectTxMsg for each message.
	QMap<QString, E2EProtectTxMsg *> e2eTxMsgMap;
	/// @brief An empty E2EProtectTxMsg instance used as a placeholder.
	E2EProtectTxMsg emptyTxMsg;
	/// @brief Timer for running the state machine.
	QTimer *workerTimerPtr;

	// functions
	/// @brief Prefixes with "E2EProtectSend: " for debug messages.
	void debug(const QString& s);
	/**
	 * @brief Checks if a message is already in the E2E list.
	 * @param msgName The name of the message to check.
	 * @return True if the message is in the E2E list, false otherwise.
	 */
	bool isInE2E(QString msgName);
	/**
	 * @brief Adds a message to the E2E list.
	 * @details This function creates a new E2EProtectTxMsg instance and adds it to the e2eTxMsgMap.
	 * @param msgName The name of the message to add.
	 */
	void addToE2E(QString msgName);
	/**
	 * @brief Removes a message from the E2E list.
	 * @details This function deletes the E2EProtectTxMsg instance associated with the message name
	 * @param msgName The name of the message to remove.
	 */
	void removeFromE2E(QString msgName);
	/**
	 * @brief Checks if the E2EProtectSend widget is closed.
	 * @param event The close event.
	 */
	void closeEvent(QCloseEvent *event);
	/**
	 * @brief Runs the state machine for each E2EProtectTxMsg instance.
	 * @details This function is called periodically by the worker timer to update the state of each message.
	 */
	void runSM(void);
};

#endif // E2EPROTECTSEND_H
