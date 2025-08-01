#ifndef UDSFLASHER_H
#define UDSFLASHER_H

#include <QMainWindow>
#include <QThread>
#include <QSettings>
#include <memory>

#include "can.h"
#include "udsflashworker.h"

namespace Ui {
class UdsFlasher;
}

class UdsFlasher : public QMainWindow
{
	Q_OBJECT

public:
	explicit UdsFlasher(QWidget *parent = nullptr);
	~UdsFlasher();
signals:
	/**
	 * @brief Signal emitted when the UDS Flasher is closed.
	 * @param ptr Pointer to the this UdsFlasher instance.
	 */
	void closed(UdsFlasher *ptr);
	/**
	 * @brief Signal emitted when a CAN message is sent.
	 * @param canMsg The CAN message that was sent.
	 */
	void sendCanMsg(const CanMsg &canMsg);

public slots:
	/**
	 * @brief Slot to handle the reception of a CAN message.
	 * @param canMsgRef The received CAN message.
	 */
	void onCanMsgReceived(const CanMsg &canMsgRef);
	void onWorkerSendCanMsg(const CanMsg &canMsg);
	void onWorkerSendLog(const QString &s);

private slots:
	void onFlashWorkerFinished(void);

	void on_flashPushButton_clicked();

	void on_binFilePushButton_clicked();

	void on_seednkeyFilePushButton_clicked();

	void on_timeoutSpinBox_valueChanged(int arg1);

	void on_addrLineEdit_textChanged(const QString &arg1);

	void on_securityLevelSpinBox_valueChanged(int arg1);

	void on_respIdLineEdit_textChanged(const QString &arg1);

	void on_reqIdLineEdit_textChanged(const QString &arg1);

private:
	Ui::UdsFlasher *ui;
	QThread *flashThreadPtr;
	UdsFlashWorker *flashWorkerPtr;
	ThreadSafeQueue<CanMsg> *rxCanMsgQueuePtr;
	std::unique_ptr<QSettings> settings;
	bool isFlashing;
	/// @brief Handles the close event of the UDS Flasher.
	/// @param event The close event.
	void closeEvent(QCloseEvent *event);
	void setHexValidators();
	uint32_t getAddr(void) const;
	uint32_t getReqId(void) const;
	uint32_t getRespId(void) const;
	bool isFlashPrecondOk(void) const;
	void setConfigUiDisabled(bool disabled);
	void flash(void);
};

#endif // UDSFLASHER_H
