#ifndef PICKUPABLE_H
#define PICKUPABLE_H

#include <QWidget>
#include <QMap>

class QMovie;

class Pickupable : public QWidget
{
Q_OBJECT
	public:
	enum states {
		stateIdle = 0,
		stateHeld,
		stateMove
	} cur_state;

	private:
	int ditzy;
	bool jumpy;

	int logic_timer;
	QPoint held_pos;

	QPointF velocity;
	qreal bounce_factor;
	qreal holdpos_time;

	QMap<QString, QMovie*> sprites;
	QMovie * cur_sprite;
	bool sprite_mirrored;

	protected:
	void movieUpdate(const QRect & rect);
	void paintEvent(QPaintEvent * event);
	void timerEvent(QTimerEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

	public:
	Pickupable(const QString & c);
	void reset();
	bool load(const QString & c);

	states state() const;
	const QString stateName() const;
	void setState(states s);
};

#endif
