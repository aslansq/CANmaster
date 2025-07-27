#ifndef E2EPROTECTSEND_H
#define E2EPROTECTSEND_H

#include <QWidget>

namespace Ui {
class E2EProtectSend;
}

class E2EProtectSend : public QWidget
{
	Q_OBJECT

public:
	explicit E2EProtectSend(QWidget *parent = nullptr);
	~E2EProtectSend();

private slots:

private:
	Ui::E2EProtectSend *ui;
};

#endif // E2EPROTECTSEND_H
