#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	checkButtonGroupPtr = new QButtonGroup(this);
	checkButtonGroupPtr->addButton(ui->stdCheckBox);
	checkButtonGroupPtr->addButton(ui->fdCheckBox);
	checkButtonGroupPtr->addButton(ui->replayCheckBox);
	checkButtonGroupPtr->setExclusive(true);

	ui->fdCheckBox->setVisible(false);
	ui->replayCheckBox->setVisible(false);

	canStdFormPtr = new CanStdForm(this);

	ui->mainVerticalLayout->addWidget(canStdFormPtr);
	ui->mainVerticalLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	ui->stdCheckBox->setCheckState(Qt::Checked);
	ui->treeWidget->expandAll();
}

MainWindow::~MainWindow()
{
	delete ui;
	delete checkButtonGroupPtr;
	delete canStdFormPtr;

	for(E2EProtectSend * e2eProtectSendPtr : e2eProtectSendPtrVect) {
		delete e2eProtectSendPtr;
	}
}


void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	qDebug() << item->text(column);

	if(item->text(column) == "E2E_Protect_Send") {
		auto e2eProtectSendPtr = new E2EProtectSend();
		e2eProtectSendPtr->setVisible(true);
		e2eProtectSendPtrVect.append(e2eProtectSendPtr);
	}

	clearUnvisibleWidgets();
}

void MainWindow::clearUnvisibleWidgets(void)
{
	QVector<E2EProtectSend *> unE2EProtectSendPtrVect;
	unE2EProtectSendPtrVect.clear();

	for(E2EProtectSend *e2eProtectSendPtr : e2eProtectSendPtrVect) {
		if(!e2eProtectSendPtr->isVisible()) {
			unE2EProtectSendPtrVect.append(e2eProtectSendPtr);
		}
	}

	for(E2EProtectSend * unE2EProtectSendPtr : unE2EProtectSendPtrVect) {
		e2eProtectSendPtrVect.removeOne(unE2EProtectSendPtr);
		delete unE2EProtectSendPtr;
	}
}


