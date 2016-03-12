#ifndef PICKUPABLE_H
#define PICKUPABLE_H

#include <QWidget>

class Pickupable : public QWidget
{
Q_OBJECT
	private:
	enum states {
		stateIdle,
		stateHeld,
		stateMoving
	} state;

	int logic_timer;
	QPoint held_pos;

	QPointF velocity;
	qreal bounce_factor;

	protected:
	void timerEvent(QTimerEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

	public:
	Pickupable(const QString & c);
	void load(const QString & c);
};

#endif
