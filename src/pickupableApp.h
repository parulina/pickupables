#ifndef PICKUPABLEAPP_H
#define PICKUPABLEAPP_H

#include <QApplication>
#include <QList>

class Pickupable;
class PickupableTray;

class PickupableApp : public QApplication
{
Q_OBJECT
	private:
	PickupableTray * tray;
	QList<Pickupable*> pickupables;

	public:
	PickupableApp(int &argc, char **argv);
};

#endif
