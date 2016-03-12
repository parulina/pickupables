#include "pickupableTray.h"

PickupableTray::PickupableTray(const QIcon & icon, QObject * parent) :
	QSystemTrayIcon(icon, parent)
{
	this->showMessage("test", "test2");
}
