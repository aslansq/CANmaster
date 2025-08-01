#ifndef UDSFLASHWORKER_H
#define UDSFLASHWORKER_H

#include <QObject>
#include <QThread>
#include <QString>
#include "can.h"
#include "isotp.hpp"
#include "seednkey.h"

class UdsFlashWorker : public QObject
{
	Q_OBJECT
public:
	explicit UdsFlashWorker(
		QObject *parent,
		ThreadSafeQueue<CanMsg> *rxCanMsgQueuePtr,
		uint32_t reqId,
		uint32_t respId,
		QString binFilePath,
		QString seednkeyFilePath,
		int securityLevel,
		uint32_t addr,
		uint32_t timeout
	);
	~UdsFlashWorker();

	// service identifiers
	static constexpr uint8_t DIAG_CTRL = 0x10;
	static constexpr uint8_t NEG_RESP = 0x7F;
	static constexpr uint8_t SECURITY_ACCESS = 0x27;
	static constexpr uint8_t REQ_DOWN = 0x34;
	static constexpr uint8_t TRANSFER_DATA = 0x36;
	static constexpr uint8_t TRANSFER_EXIT = 0x37;
	static constexpr uint8_t ECU_RESET = 0x11;

	static constexpr uint8_t SID_POS = 0;
	static constexpr uint8_t SUB_FUNC_POS = 1;

	// diagnostic session control sub-functions
	static constexpr uint8_t DIAG_CTRL_SUB_FUNC_PROG = 0x02;

	// security access sub-functions
	static constexpr uint8_t SECURITY_ACCESS_REQ_SEED = 0x03;
	static constexpr uint8_t SECURITY_ACCESS_SEND_KEY = 0x04;

	// reset sub-functions
	static constexpr uint8_t ECU_RESET_HARD = 0x01;

	// positive response offset
	static constexpr uint8_t POS_RESP_SID_OFFSET = 0x40;

	// request down sub-functions
	static constexpr uint8_t POS_REQ_DOWN_DATA_FORMAT_ID = 1;
	static constexpr uint8_t POS_REQ_DOWN_ADDR_LEN_FORMAT_ID = 2;
	static constexpr uint8_t POS_REQ_DOWN_MEM_ADDR = 3;

public slots:
	void run();

signals:
	void finished(void);
	void sendCanMsg(const CanMsg &canMsg);
	void sendLog(const QString &s);

private:
	enum class WorkResult {
		OK,
		NRC,
		SEEDNKEY_NOT_FOUND,
		BIN_NOT_FOUND,
		KEYGEN_NOT_FOUND,
		PROG_SESSION_FAILED,
		SEED_REQ_FAILED,
		SEND_KEY_FAILED,
		ISOTP_PACKET_FAILED,
		REQ_DOWN_FAILED,
		TRANSFER_DATA_FAILED,
		REQ_TRANSFER_EXIT_FAILED,
		HARD_RESET_FAILED
	};

	ThreadSafeQueue<CanMsg> *rxCanMsgQueuePtr;
	uint32_t reqId;
	uint32_t respId;
	QString binFilePath;
	QString seednkeyFilePath;
	int securityLevel;
	uint32_t addr;
	uint32_t timeout;
	QVector<CanMsg> canMsgVect;
	IsoTp *isoTpPtr;
	uint8_t sendBuf[4096];
	uint8_t recvBuf[4096];
	void* keyGenHandlePtr;
	QVector<uint8_t> seed;
	GenerateKeyExOptFunc keyGenFuncPtr;
	int maxBlockLen;

	void readBinFile(const QString &fileName, QVector<uint8_t> &outData) const;
	WorkResult checkFilesExist();
	WorkResult checkNegResponse(const CanMsg &resp, uint8_t expectedService);
	WorkResult checkNAssignKeyGen(void);
	QVector<uint8_t> convertCanMsgToQVector(const CanMsg &canMsg) const;

	WorkResult getIsoTpPacket(
		uint8_t *data,
		uint16_t dataLength,
		uint16_t *rxLen
	);
	WorkResult progSessionReq(void);
	WorkResult seedReq(void);
	WorkResult sendKey(void);
	WorkResult reqDown(void);
	WorkResult transferData(void);
	WorkResult reqTransferExit(void);
	WorkResult hardReset(void);
};

#endif // UDSFLASHWORKER_H
