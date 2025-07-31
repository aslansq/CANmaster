/**
 * @file e2ereceivecheckp11.h
 * @brief This file defines the E2EReceiveCheckP11 class, which is used
 *        for managing the E2E P11 receive check configuration gui elements.
 * @details The E2EReceiveCheckP11 class provides methods to set and get P11 related configurations,
 *          such as Data ID mode and maximum delta counter.
 */

#ifndef E2ERECEIVECHECKP11_H
#define E2ERECEIVECHECKP11_H

#include <QWidget>

namespace Ui {
class E2EReceiveCheckP11;
}

class E2EReceiveCheckP11 : public QWidget
{
	Q_OBJECT

public:
	enum class DataIdMode {
		BOTH = 0,
		NIBBLE
	};

	explicit E2EReceiveCheckP11(QWidget *parent = nullptr);
	~E2EReceiveCheckP11();
	DataIdMode getDataIdMode(void) const;
	uint16_t getDataId(void) const;
	uint32_t getMaxDeltaCounter(void) const;
	void setDataId(uint16_t id);
	void setDataIdMode(DataIdMode mode);

private slots:

private:
	Ui::E2EReceiveCheckP11 *ui;
};

#endif // E2ERECEIVECHECKP11_H
