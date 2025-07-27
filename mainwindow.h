#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QTreeWidgetItem>
#include <QVector>
#include <QPointer>
#include "canstdform.h"
#include "e2eprotectsend.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
	Ui::MainWindow *ui;
	QButtonGroup *checkButtonGroupPtr;
	CanStdForm *canStdFormPtr;
	QVector<E2EProtectSend *> e2eProtectSendPtrVect;

	void clearUnvisibleWidgets(void);
};
#endif // MAINWINDOW_H
