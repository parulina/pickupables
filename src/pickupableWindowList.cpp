#include "pickupableWindowList.h"

#include <QDebug>

#ifdef Q_OS_UNIX
 #include <QX11Info>
 #include <X11/Xatom.h>
 #include <X11/Xlib.h>
#endif

QList<windowInfo> PickupableWindowList::windowGeometryList()
{
	QList<windowInfo> window_list;

#ifdef Q_OS_UNIX
		if(QX11Info::isPlatformX11()){
			Display * display = QX11Info::display();
			int rootWindow = QX11Info::appRootWindow();

			Atom a = XInternAtom(display, "_NET_CLIENT_LIST" , true);
			Atom b = XInternAtom(display, "_NET_WM_STATE" , true);
			Atom state_maximized_vertical = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", true);
			Atom state_maximized_horizontal = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", true);

			Atom actualType;
			int format;
			unsigned long numItems, bytesAfter;
			unsigned char *data =0;
			int status = XGetWindowProperty(display, rootWindow,
					a, 0L, (~0L), false, AnyPropertyType, &actualType,
					&format, &numItems, &bytesAfter, &data);

			if (status >= Success && numItems) {
				Q_ASSERT(format == 32);
				quint32 *array = (quint32*) data;
				for (quint32 k = 0; k < numItems; k++) {
					Window w = (Window) array[k];
					Window rootWindow;
					int x, y;
					int m = 0;
					unsigned int width, height;
					unsigned int x1, x2;

					status = XGetGeometry(display, w, &rootWindow, &x, &y, &width, &height, &x1, &x2);
					if(!status) continue;

					status = XTranslateCoordinates(display, w, rootWindow, x, y, &x, &y, &rootWindow);
					if(!status) continue;

					int format2;
					unsigned long numStates = 0, bytesAfter2;
					Atom actualType2;
					Atom * stateData = 0;
					status = XGetWindowProperty(display, w, b, 0, 1024, false, XA_ATOM,
							&actualType2, &format2, &numStates, &bytesAfter2, (unsigned char**)&stateData);
					if(status >= Success && numStates){
						for(int i = 0; i < numStates; i++){
							if(stateData[i] == state_maximized_vertical || stateData[i] == state_maximized_horizontal) {
								m++;
							}
						}
					}

					windowInfo info;
					info.rect = QRect(QPoint(x, y), QSize(width, height));
					info.maximized = (m == 2);
					window_list << info;
				}
				XFree(data);
			}
		}
#endif

	return window_list;
}
