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

	uint64_t getBaud(void) const;
	int getChannel(void) const;
	QString getInterface(void);

private slots:
	void on_channelComboBox_currentTextChanged(const QString &arg1);

	void on_baudComboBox_currentTextChanged(const QString &arg1);

private:
	Ui::CanStdForm *ui;
	QString baud;
	QString channel;
	QMap<QString, int> availableBaudrates;
	QMap<QString, int> availableDevChannels;
};

#endif // CANSTDFORM_H
