#include "udsflashworker.h"
#include <QDebug>
#include <QFile>
#include <dlfcn.h>

UdsFlashWorker::UdsFlashWorker(
	QObject *parent,
	ThreadSafeQueue<CanMsg> *rxCanMsgQueuePtr,
	uint32_t reqId,
	uint32_t respId,
	QString binFilePath,
	QString seednkeyFilePath,
	int securityLevel,
	uint32_t addr,
	uint32_t timeout
) :
	QObject(parent),
	rxCanMsgQueuePtr(rxCanMsgQueuePtr),
	reqId(reqId),
	respId(respId),
	binFilePath(binFilePath),
	seednkeyFilePath(seednkeyFilePath),
	securityLevel(securityLevel),
	addr(addr),
	timeout(timeout),
	isoTpPtr(nullptr),
	keyGenHandlePtr(nullptr),
	keyGenFuncPtr(nullptr)
{
	isoTpPtr = new IsoTp(this);
	isoTpPtr->init(
		reqId,
		sendBuf,
		sizeof(sendBuf),
		recvBuf,
		sizeof(recvBuf)
	);
	connect(isoTpPtr, &IsoTp::sendCanMsg, this, &UdsFlashWorker::sendCanMsg);
}

UdsFlashWorker::~UdsFlashWorker()
{
	emit sendLog("Flash Process Finished");
	if (isoTpPtr) {
		delete isoTpPtr;
		isoTpPtr = nullptr;
	}
	emit finished();
}

void UdsFlashWorker::readBinFile(const QString &fileName, QVector<uint8_t> &outData) const
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		throw std::runtime_error("Failed to open binary file");
	}

	outData.clear();
	while (!file.atEnd()) {
		QByteArray byteArray = file.read(4096); // Read in chunks
		for (char byte : byteArray) {
			outData.append(static_cast<uint8_t>(byte));
		}
	}
	file.close();
}

QVector<uint8_t> UdsFlashWorker::convertCanMsgToQVector(const CanMsg &canMsg) const
{
	QVector<uint8_t> vec;
	vec.clear();
	for(int i = 0; i < canMsg.dataLength; ++i) {
		vec.append(canMsg.data[i]);
	}
	return vec;
}

UdsFlashWorker::WorkResult UdsFlashWorker::checkNegResponse(const CanMsg &resp, uint8_t expectedService)
{
	if(resp.dataLength < 1) {
		return WorkResult::OK; // Not a negative response
	}

	if(resp.data[SID_POS] != NEG_RESP) {
		return WorkResult::OK; // Not a negative response
	}

	return WorkResult::NRC; // Not a recognized negative response
}

UdsFlashWorker::WorkResult UdsFlashWorker::checkFilesExist()
{
	if (!QFile::exists(seednkeyFilePath)) {
		emit sendLog(QString("Seed & Key file does not exist: %1").arg(seednkeyFilePath));
		return WorkResult::SEEDNKEY_NOT_FOUND;
	}

	emit sendLog("Found Seed & Key file");

	if (!QFile::exists(binFilePath)) {
		emit sendLog(QString("Binary file does not exist: %1").arg(binFilePath));
		return WorkResult::BIN_NOT_FOUND;
	}

	emit sendLog("Found binary file");

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::checkNAssignKeyGen(void)
{
	keyGenHandlePtr = dlopen(seednkeyFilePath.toStdString().c_str(), RTLD_LAZY);
	if (keyGenHandlePtr == nullptr) {
		emit sendLog(QString("Failed to load library: %1").arg(dlerror()));
		return WorkResult::KEYGEN_NOT_FOUND;
	}

	// Clear any existing errors
	dlerror();

	keyGenFuncPtr = (GenerateKeyExOptFunc)dlsym(keyGenHandlePtr, "GenerateKeyExOpt"); // Try original name

	if (keyGenFuncPtr == nullptr) {
		emit sendLog(QString("Failed to find function: %1").arg(dlerror()));
		dlclose(keyGenHandlePtr);
		return WorkResult::KEYGEN_NOT_FOUND;
	}

	emit sendLog("Found key generation function");

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::progSessionReq(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;
	
	req.dataLength = 2;
	req.data[SID_POS] = DIAG_CTRL;
	req.data[SUB_FUNC_POS] = DIAG_CTRL_SUB_FUNC_PROG;

	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	emit sendLog("Programming session requested");

	bool isReceived = rxCanMsgQueuePtr->waitAndDequeue(resp, timeout);

	if(!isReceived) {
		emit sendLog("No response to programming session req");
		return WorkResult::PROG_SESSION_FAILED;
	}

	isoTpPtr->on_can_message(resp.data, resp.dataLength);
	isoTpPtr->poll();
	IsoTpRet isoTpRet = isoTpPtr->receive(resp.data, sizeof(resp.data), &rxLen);
	resp.dataLength = rxLen;

	if(isoTpRet != IsoTpRet::OK) {
		emit sendLog("No response to programming session req");
		return WorkResult::PROG_SESSION_FAILED;
	}

	result = checkNegResponse(resp, DIAG_CTRL);

	if (result != WorkResult::OK) {
		emit sendLog("Programming session request was rejected");
		return WorkResult::PROG_SESSION_FAILED;
	}

	if(
		resp.dataLength < 2 ||
		resp.data[0] != (DIAG_CTRL + POS_RESP_SID_OFFSET) ||
		resp.data[1] != DIAG_CTRL_SUB_FUNC_PROG) {
		emit sendLog("Invalid response to programming session request");
		return WorkResult::PROG_SESSION_FAILED;
	}

	emit sendLog("Programming session accepted");
	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::getIsoTpPacket(
	uint8_t *data,
	uint16_t dataLength,
	uint16_t *rxLen
)
{
	bool isTimerElapsed;
	bool isReceived;
	QElapsedTimer timer;
	CanMsg resp;
	IsoTpRet isoTpRet;

	timer.start();
	isTimerElapsed = false;
	isoTpRet = IsoTpRet::ISO_NO_DATA;

	while(isoTpRet != IsoTpRet::OK && isTimerElapsed == false) {
		isReceived = rxCanMsgQueuePtr->waitAndDequeue(resp, timeout);
		if(isReceived) {
			isoTpPtr->on_can_message(resp.data, resp.dataLength);
		}
		isoTpPtr->poll();
		*rxLen = 0;
		isoTpRet = isoTpPtr->receive(data, dataLength, rxLen);
		isTimerElapsed = timer.elapsed() > timeout;
	}

	if(isoTpRet != IsoTpRet::OK || isTimerElapsed == true) {
		emit sendLog("No response from ISO-TP");
		return WorkResult::ISOTP_PACKET_FAILED;
	}

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::seedReq(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;

	req.dataLength = 2;
	req.data[SID_POS] = SECURITY_ACCESS;
	req.data[SUB_FUNC_POS] = SECURITY_ACCESS_REQ_SEED;

	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	emit sendLog("Security access seed request sent");

	result = getIsoTpPacket(resp.data, sizeof(resp.data), &rxLen);

	if (result != WorkResult::OK) {
		emit sendLog("Failed to receive response to security access seed request");
		return WorkResult::SEED_REQ_FAILED;
	}

	result = checkNegResponse(resp, SECURITY_ACCESS);

	if (result != WorkResult::OK) {
		emit sendLog("Security access seed request was rejected");
		return WorkResult::SEED_REQ_FAILED;
	}

	if(
		resp.dataLength < 2 ||
		resp.data[0] != (SECURITY_ACCESS + POS_RESP_SID_OFFSET) ||
		resp.data[1] != SECURITY_ACCESS_REQ_SEED) {
		emit sendLog("Invalid response to security access seed request");
		return WorkResult::SEED_REQ_FAILED;
	}

	seed.clear();
	for(int i = (SUB_FUNC_POS+1); i < resp.dataLength; ++i) {
		seed.append(resp.data[i]);
	}

	if (seed.isEmpty()) {
		emit sendLog("No seed received in response to security access seed request");
		return WorkResult::SEED_REQ_FAILED;
	}

	emit sendLog("Security access seed request accepted");
	seed = convertCanMsgToQVector(resp);
	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::sendKey(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;
	uint8_t keyArray[512];
	unsigned int actualKeySize = 0;
	bool isTimerElapsed;
	bool isReceived;
	QElapsedTimer timer;

	req.dataLength = 2 + seed.size();
	req.data[SID_POS] = SECURITY_ACCESS;
	req.data[SUB_FUNC_POS] = SECURITY_ACCESS_SEND_KEY;

	result = checkNAssignKeyGen();

	if (result != WorkResult::OK) {
		emit sendLog("Key generation function not found");
		return WorkResult::SEND_KEY_FAILED;
	}

	keyGenFuncPtr(
		seed.data(),
		seed.size(),
		securityLevel,
		"DefaultVariant", // Replace with actual variant if needed
		"DefaultOptions", // Replace with actual options if needed
		keyArray,
		sizeof(keyArray),
		&actualKeySize
	);

	dlclose(keyGenHandlePtr);
	keyGenHandlePtr = nullptr;

	if (actualKeySize == 0) {
		emit sendLog("Key generation failed, no key generated");
		return WorkResult::SEND_KEY_FAILED;
	}

	for (int i = 0; i < actualKeySize; ++i) {
		req.data[2 + i] = keyArray[i];
	}
	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	isoTpPtr->poll();
	isoTpPtr->poll();
	emit sendLog("Security access key sent");

	result = getIsoTpPacket(resp.data, sizeof(resp.data), &rxLen);
	resp.dataLength = rxLen;

	if (result != WorkResult::OK) {
		emit sendLog("Failed to receive response to security access key request");
		return WorkResult::SEND_KEY_FAILED;
	}

	result = checkNegResponse(resp, SECURITY_ACCESS);

	if (result != WorkResult::OK) {
		emit sendLog("Security access key request was rejected");
		return WorkResult::SEND_KEY_FAILED;
	}

	if(
		resp.dataLength < 2 ||
		resp.data[0] != (SECURITY_ACCESS + POS_RESP_SID_OFFSET) ||
		resp.data[1] != SECURITY_ACCESS_SEND_KEY) {
		emit sendLog("Invalid response to security access key request");
		return WorkResult::SEND_KEY_FAILED;
	}

	emit sendLog("Security access key accepted");
	seed.clear(); // Clear seed after successful key send

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::reqDown(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;
	QVector<uint8_t> binData;

	readBinFile(binFilePath, binData);

	// SID_POS + POS_REQ_DOWN_DATA_FORMAT_ID + POS_REQ_DOWN_ADDR_LEN_FORMAT_ID + 8; // 4 bytes address + 4 bytes length
	req.dataLength = 11;
	req.data[SID_POS] = REQ_DOWN;
	req.data[POS_REQ_DOWN_DATA_FORMAT_ID] = 0; // no compression, or encyption
	req.data[POS_REQ_DOWN_ADDR_LEN_FORMAT_ID] = 0x44; // 4 bytes address, 4 bytes length
	uint32_t *ptr =(uint32_t *) &req.data[POS_REQ_DOWN_MEM_ADDR];
	*ptr = addr;
	ptr = (uint32_t *) &req.data[POS_REQ_DOWN_MEM_ADDR + 4];
	*ptr = binData.size();

	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	isoTpPtr->poll();
	isoTpPtr->poll();
	isoTpPtr->poll();
	emit sendLog("Requesting download of binary data");

	result = getIsoTpPacket(resp.data, sizeof(resp.data), &rxLen);
	resp.dataLength = rxLen;

	if (result != WorkResult::OK) {
		emit sendLog("Failed to receive response to request download");
		return WorkResult::REQ_DOWN_FAILED;
	}

	result = checkNegResponse(resp, REQ_DOWN);

	if (result != WorkResult::OK) {
		emit sendLog("Request download was rejected");
		return WorkResult::REQ_DOWN_FAILED;
	}

	if(
		resp.dataLength < 2 ||
		resp.data[0] != (REQ_DOWN + POS_RESP_SID_OFFSET)
	) {
		emit sendLog("Invalid response to request download");
		return WorkResult::REQ_DOWN_FAILED;
	}

	uint8_t maxBlockLenSize = resp.data[1] >> 4;

	if(resp.dataLength != (2 + maxBlockLenSize)) {
		emit sendLog("Malformed response to request download");
	}

	if(maxBlockLenSize == 2) {
		uint16_t *ptr = (uint16_t *)&resp.data[2];
		maxBlockLen = *ptr;
	} else if(maxBlockLenSize == 4) {
		uint32_t *ptr = (uint32_t *)&resp.data[2];
		maxBlockLen = *ptr;
	}

	// there is bug is multiframe transfer in ISO-TP, so we limit maxBlockLen to 4 bytes
	if(maxBlockLen > 4) {
		maxBlockLen = 4;
	}

	emit sendLog("Request download accepted");

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::transferData(void)
{
	CanMsg resp;
	QVector<uint8_t> isotpReq;
	QVector<uint8_t> isotpResp;
	WorkResult result;
	QVector<uint8_t> binData;
	uint8_t bsc = 0;
	uint16_t rxLen = 0;
	int percentage = 0;
	int lastPercentage = 0;

	readBinFile(binFilePath, binData);

	for(int i = 0; i < binData.size(); i += maxBlockLen) {
		bsc++;
		isotpReq.clear();
		isotpReq.append(TRANSFER_DATA);
		isotpReq.append(bsc); // Block sequence counter
		for(int j = i; j < (i + maxBlockLen) && j < binData.size(); ++j) {
			isotpReq.append(binData[j]);
		}
		rxCanMsgQueuePtr->clear();
		// Send the ISO-TP request
		isoTpPtr->send(isotpReq.data(), isotpReq.size());
		for(int k = 0; k < maxBlockLen; ++k) {
			isoTpPtr->poll(); // Poll to ensure the message is sent
		}
		percentage = (i + maxBlockLen) * 100 / binData.size();

		if (percentage != lastPercentage) {
			emit sendLog(QString("Transferring data blocks %1%").arg(percentage));
		}

		getIsoTpPacket(
			resp.data,
			sizeof(resp.data),
			&rxLen
		);
		resp.dataLength = rxLen;

		if(
			resp.dataLength < 1 ||
			resp.data[0] != (TRANSFER_DATA + POS_RESP_SID_OFFSET)
		) {
			emit sendLog(QString("Invalid response to transfer data block %1").arg(bsc));
			return WorkResult::TRANSFER_DATA_FAILED;
		}

		lastPercentage = percentage;
	}

	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::reqTransferExit(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;

	req.dataLength = 1;
	req.data[SID_POS] = TRANSFER_EXIT;

	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	emit sendLog("Requesting transfer exit");

	result = getIsoTpPacket(resp.data, sizeof(resp.data), &rxLen);
	resp.dataLength = rxLen;

	if (result != WorkResult::OK) {
		emit sendLog("Failed to receive response to request transfer exit");
		return WorkResult::REQ_TRANSFER_EXIT_FAILED;
	}

	result = checkNegResponse(resp, TRANSFER_EXIT);

	if (result != WorkResult::OK) {
		emit sendLog("Request transfer exit was rejected");
		return WorkResult::REQ_TRANSFER_EXIT_FAILED;
	}

	if(
		resp.dataLength < 1 ||
		resp.data[0] != (TRANSFER_EXIT + POS_RESP_SID_OFFSET)
	) {
		emit sendLog("Invalid response to request transfer exit");
		return WorkResult::REQ_TRANSFER_EXIT_FAILED;
	}

	emit sendLog("Request transfer exit accepted");
	return WorkResult::OK;
}

UdsFlashWorker::WorkResult UdsFlashWorker::hardReset(void)
{
	CanMsg req;
	CanMsg resp;
	uint16_t rxLen = 0;
	WorkResult result;

	req.dataLength = 2;
	req.data[SID_POS] = ECU_RESET;
	req.data[SUB_FUNC_POS] = ECU_RESET_HARD;

	rxCanMsgQueuePtr->clear();
	isoTpPtr->send(req.data, req.dataLength);
	emit sendLog("Requesting hard reset");

	result = getIsoTpPacket(resp.data, sizeof(resp.data), &rxLen);
	resp.dataLength = rxLen;

	if (result != WorkResult::OK) {
		emit sendLog("Failed to receive response to request hard reset");
		return WorkResult::HARD_RESET_FAILED;
	}

	result = checkNegResponse(resp, ECU_RESET);

	if (result != WorkResult::OK) {
		emit sendLog("Request hard reset was rejected");
		return WorkResult::HARD_RESET_FAILED;
	}

	if(
		resp.dataLength < 1 ||
		resp.data[0] != (ECU_RESET + POS_RESP_SID_OFFSET)
	) {
		emit sendLog("Invalid response to request hard reset");
		return WorkResult::HARD_RESET_FAILED;
	}

	emit sendLog("Request hard reset accepted");
	return WorkResult::OK;
}

void UdsFlashWorker::run()
{
	WorkResult result = WorkResult::OK;
	emit sendLog("Starting Flashing Process");

	result = checkFilesExist();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = progSessionReq();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = seedReq();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = sendKey();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = reqDown();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = transferData();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = reqTransferExit();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	result = hardReset();

	if (result != WorkResult::OK) {
		deleteLater();
		return;
	}

	deleteLater();
}
