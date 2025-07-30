#ifndef PEAKBASICCAN_H
#define PEAKBASICCAN_H

#include <QMap>
#include <QString>
#include "PCANBasic.h"

class PeakBasicCan
{
public:
	const TPCANHandle invalidPcanHandle = 0;
	const TPCANBaudrate invalidPcanBaud = 0;
	explicit PeakBasicCan(void);
	QString getStatusStr(TPCANStatus st);
	TPCANHandle getPeakHandle(int channelNum);
private:
	const QMap<TPCANStatus, QString> statusStrings;
};

#endif // PEAKBASICCAN_H
