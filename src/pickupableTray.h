#ifndef PICKUPABLETRAY_H
#define PICKUPABLETRAY_H

#include <QSystemTrayIcon>

class PickupableTray : public QSystemTrayIcon
{
Q_OBJECT
	public:
	PickupableTray(const QIcon & icon, QObject * = nullptr);
};

#endif
