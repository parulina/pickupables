#ifndef PICKUPABLEWINDOWLIST_H
#define PICKUPABLEWINDOWLIST_H

#include <QList>
#include <QRect>

struct windowInfo {
	QRect rect;
	bool maximized;
};

class PickupableWindowList
{
	public:
	static QList<windowInfo> windowGeometryList();
};

#endif
