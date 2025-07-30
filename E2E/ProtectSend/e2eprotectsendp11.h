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
	void setDisabled(bool isDisabled);

private:
	Ui::E2EProtectSendP11 *ui;
};

#endif // E2EPROTECTSENDP11_H
