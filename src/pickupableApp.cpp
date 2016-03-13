#include "pickupableApp.h"

#include "pickupable.h"
#include "pickupableTray.h"

#include <QScreen>
#include <QDebug>

#ifndef VERSION
	#define VERSION "0.0.0"
#endif

PickupableApp::PickupableApp(int & argc, char **argv) :
	QApplication(argc, argv)
{
	QIcon icon(":icon.png");
	this->setWindowIcon(icon);

	this->setOrganizationName("paru");
	this->setOrganizationDomain("sqnya.se");

	this->setApplicationName("pickupables");
	this->setApplicationVersion(VERSION);

	qDebug("Initializing [%s %s]", this->applicationName().toStdString().c_str(), VERSION);

	tray = new PickupableTray(icon, this);
	connect(tray, &PickupableTray::activated, this, &PickupableApp::quit);
	tray->show();

	this->startTimer(1000/60);

	pickupables << new Pickupable("test.zip");
}
