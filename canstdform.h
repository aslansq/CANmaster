#ifndef CANSTDFORM_H
#define CANSTDFORM_H

#include <QWidget>
#include <QMap>

namespace Ui {
class CanStdForm;
}

class CanStdForm : public QWidget
{
	Q_OBJECT

public:
	explicit CanStdForm(QWidget *parent = nullptr);
	~CanStdForm();

private:
	Ui::CanStdForm *ui;
	QMap<QString, int> availableBaudrates;
	QMap<QString, int> availableDevChannels;
};

#endif // CANSTDFORM_H
