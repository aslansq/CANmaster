/**
 * @file e2ereceivecheck.h
 * @brief Main implementation file for E2E receive check functionality.
 * @details This file defines the E2EReceiveCheck class, which manages the reception and
 *          checking of E2E protected messages in a CAN network.
 */

#ifndef E2ERECEIVECHECK_H
#define E2ERECEIVECHECK_H

#include <QWidget>

#include "can.h"
#include "e2ereceiverxmsg.h"

namespace Ui {
class E2EReceiveCheck;
}

class E2EReceiveCheck : public QWidget
{
	Q_OBJECT

public:
	/**
	 * @brief reads the DBC network and initializes the E2EReceiveCheck widget with CAN messages.
	 * @param netPtr A shared pointer to the DBC network.
	 */
	E2EReceiveCheck(std::shared_ptr<dbcppp::INetwork> netPtr);
	~E2EReceiveCheck();
public slots:
	/**
	 * @brief Slot to handle the reception of a CAN message.
	 * @param canMsgRef The received CAN message.
	 */
	void onCanMsgReceived(const CanMsg &canMsgRef);

signals:
	/**
	 * @brief Signal emitted when user presses the close button.
	 * @param ptr Pointer to the this E2EReceiveCheck instance.
	 */
	void closed(E2EReceiveCheck *ptr);
	/**
	 * @brief Signal emitted when a CAN message is received. This is used to notify E2EReceiveRxMsg instances.
	 * @param canMsg The CAN message that was received.
	 */
	void receivedCanMsg(const CanMsg &canMsg);

private slots:
	void on_addPushButton_clicked();

	void on_e2eTableWidget_cellClicked(int row, int column);

private:
	Ui::E2EReceiveCheck *ui;
	/// @brief Pointer to the DBC network.
	std::shared_ptr<dbcppp::INetwork> netPtr;
	/// @brief List of E2EReceiveRxMsg widgets that display received E2E messages.
	QMap<QString, E2EReceiveRxMsg *> e2eRxMsgMap;
	/// @brief Empty placeholder if there are no E2E messages chosen.
	E2EReceiveRxMsg emptyRxMsg;

	bool isInE2E(QString msgName);
	void addToE2E(QString msgName);
	void removeFromE2E(QString msgName);
	void closeEvent(QCloseEvent *event);
	void debug(QString msg);
};

#endif // E2ERECEIVECHECK_H
