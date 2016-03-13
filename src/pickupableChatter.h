#ifndef PICKUPABLECHATTER_H
#define PICKUPABLECHATTER_H

#include <QWidget>
class QVBoxLayout;

class PickupableChatter : public QWidget
{
	private:
	QVBoxLayout * layout;

	protected:
	void resizeEvent(QResizeEvent * event);

	public:
	PickupableChatter(QWidget * = nullptr);

	void chat(const QString & message);
};

#endif
