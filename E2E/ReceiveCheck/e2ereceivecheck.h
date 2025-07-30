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
	explicit E2EReceiveCheck(QWidget *parent = nullptr);
	E2EReceiveCheck(std::shared_ptr<dbcppp::INetwork> netPtr);
	~E2EReceiveCheck();
public slots:
	void onCanMsgReceived(const CanMsg &canMsgRef);

signals:
	void closed(E2EReceiveCheck *ptr);
	void receivedCanMsg(const CanMsg &canMsg);

private slots:
	void on_addPushButton_clicked();

	void on_e2eTableWidget_cellClicked(int row, int column);

private:
	Ui::E2EReceiveCheck *ui;
	std::shared_ptr<dbcppp::INetwork> netPtr;
	QMap<QString, E2EReceiveRxMsg *> e2eRxMsgMap;
	E2EReceiveRxMsg emptyRxMsg;

	bool isInE2E(QString msgName);
	void addToE2E(QString msgName);
	void removeFromE2E(QString msgName);
	void closeEvent(QCloseEvent *event);
	void debug(QString msg);
};

#endif // E2ERECEIVECHECK_H
