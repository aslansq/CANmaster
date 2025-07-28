#ifndef PEAKSTDCAN_H
#define PEAKSTDCAN_H

#include <QObject>

#include "can.h"
#include "peakbasiccan.h"

class PeakStdCan : public Can, public PeakBasicCan
{
	Q_OBJECT
public:
	explicit PeakStdCan(QObject *parent = nullptr);
	~PeakStdCan() override;

	void connect(std::variant<CanStdConfig, CanFdConfig> config) override;
	void disconnect(void) override;

private:
	void rx(void) override;
	void tx(void) override;
	TPCANHandle pcanHandle;
	TPCANBaudrate pcanBaud;
	void peakStdMsgToCanMsg(
		const TPCANMsg &peakMsgRef,
		TPCANTimestamp peakTimestamp,
		CanMsg &canMsgRef
	);
	void canMsgToPeakStdMsg(
		const CanMsg &canMsgRef,
		TPCANMsg &peakMsgRef
	);
	TPCANBaudrate getPcanBaud(int baudrate);

};

#endif // PEAKSTDCAN_H
