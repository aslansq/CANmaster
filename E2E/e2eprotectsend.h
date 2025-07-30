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
	E2EProtectSend(QWidget *parent = nullptr);
	E2EProtectSend(std::shared_ptr<dbcppp::INetwork> netPtr);
	~E2EProtectSend();
signals:
	void closed(E2EProtectSend *ptr);
	void sendCanMsg(CanMsg canMsg);

private slots:

	void on_addPushButton_clicked();

	void on_removePushButton_clicked();

	void on_e2eTableWidget_cellClicked(int row, int column);

	void onTxSendCanMsg(CanMsg canMsg);

private:
	Ui::E2EProtectSend *ui;
	std::shared_ptr<dbcppp::INetwork> netPtr;
	QMap<QString, E2EProtectTxMsg *> e2eTxMsgMap;
	E2EProtectTxMsg emptyTxMsg;
	QTimer *workerTimerPtr;

	// functions
	void debug(const QString& s);
	bool isInE2E(QString msgName);
	void addToE2E(QString msgName);
	void removeFromE2E(QString msgName);
	void closeEvent(QCloseEvent *event);
	void runSM(void);
};

#endif // E2EPROTECTSEND_H
