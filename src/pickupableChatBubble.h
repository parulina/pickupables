#ifndef PICKUPABLECHATBUBBLE_H
#define PICKUPABLECHATBUBBLE_H

#include <QLabel>

class PickupableChatBubble : public QLabel
{
	//protected:
	//void paintEvent(QPaintEvent * event);

	public:
	PickupableChatBubble(const QString & text, QWidget * = nullptr);
};


#endif
