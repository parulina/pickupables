#ifndef PICKUPABLE_H
#define PICKUPABLE_H

#include <QWidget>
#include <QMap>

class PickupableChatter;
class PSprite;

class Pickupable : public QWidget
{
Q_OBJECT
	private:
	PickupableChatter * chatter;

	// timer stuff
	int windowlist_timer;
	int idle_timer;
	int logic_timer;

	// important vars
	QPointF velocity;
	enum directions {
		directionLeft,
		directionRight
	} cur_direction;
	enum states {
		stateIdle = 0,
		stateHeld,
		stateMove
	} cur_state;
	enum idleStates {
		// gets increasingly more active
		stateStill = 0, // completely still
		stateSitting, // still action 1
		stateSleeping, // still action 2
		stateWalkingAround, // side to side
		stateChasingPointer, // chasing
		stateJumping, // jumping at one spot
		stateMax
	} cur_idlestate;
	int on_ground;

	QMap<QString, PSprite*> sprites;
	PSprite * cur_sprite;

	// calc vars
	QPoint held_pos;
	int ditzy;
	int idle_timeout;
	bool jumpy;

	// config
	qreal bounce_factor;
	qreal holdpos_time;
	int idle_level;

	protected:
	void paintEvent(QPaintEvent * event);
	void moveEvent(QMoveEvent * event);
	void timerEvent(QTimerEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);

	void idleStateUpdate();

	public:
	Pickupable(const QString & c);
	void reset();
	bool load(const QString & c);

	states state() const;
	const QString stateName() const;
	void setState(states s);

	directions direction() const;
	void setDirection(directions d);
};

#endif
