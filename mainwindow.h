#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QTreeWidgetItem>
#include <QVector>
#include <QPointer>
#include "canstdform.h"
#include "e2eprotectsend.h"
#include "e2ereceivecheck.h"
#include "can.h"
#include <dbcppp/Network.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void on_dbcPushButton_clicked();

	void e2eProtectSendClosed(E2EProtectSend *ptr);
	void e2eReceiveCheckClosed(E2EReceiveCheck *ptr);
	void e2eProtectSendCanMsg(CanMsg canMsg);

	void on_connectPushButton_clicked(bool checked);

	void onCanEventOccured(CanEvent event);

signals:
	void canMsgReceived(const CanMsg &canMsgRef);

private:
	enum class CanInt {
		Std,
		Fd,
		Replay
	};

	Ui::MainWindow *ui;
	QButtonGroup *checkButtonGroupPtr;
	CanStdForm *canStdFormPtr;
	QVector<E2EProtectSend *> e2eProtectSendPtrVect;
	QVector<E2EReceiveCheck *> e2eReceiveCheckPtrVect;
	std::shared_ptr<dbcppp::INetwork> netPtr;
	std::shared_ptr<Can> canPtr;

	bool isAllDbcRelatedWinClosed(void) const;
	CanInt getCanInt(void) const;
	void canStdConnect(void);
	void createE2EProtectSend(void);
	void createE2EReceiveCheck(void);
};
#endif // MAINWINDOW_H
