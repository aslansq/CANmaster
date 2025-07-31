/**
 * @file e2eprotectsendp11.h
 * @brief This file defines the E2EProtectSendP11 class, which is used
 *        for managing the E2E P11 protection configuration gui elements.
 * @details The E2EProtectSendP11 class provides methods to set and get P11 related configurations,
 */
#ifndef E2EPROTECTSENDP11_H
#define E2EPROTECTSENDP11_H

#include <QWidget>

namespace Ui {
class E2EProtectSendP11;
}

class E2EProtectSendP11 : public QWidget
{
	Q_OBJECT

public:
	enum class DataIdMode {
		BOTH = 0,
		NIBBLE
	};

	explicit E2EProtectSendP11(QWidget *parent = nullptr);
	~E2EProtectSendP11();
	DataIdMode getDataIdMode(void) const;
	uint16_t getDataId(void) const;
	void setDataId(uint16_t id);
	void setDataIdMode(DataIdMode mode);
	/**
	 * @brief Enable or disable the P11 related GUI elements.
	 *        When messages are being sent, the GUI elements should be disabled.
	 * @param isDisabled If true, the GUI elements will be disabled.
	 */
	void setDisabled(bool isDisabled);

private:
	Ui::E2EProtectSendP11 *ui;
};

#endif // E2EPROTECTSENDP11_H
