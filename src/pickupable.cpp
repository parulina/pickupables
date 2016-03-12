#include "pickupable.h"

#include <QWindow>
#include <QScreen>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>

Pickupable::Pickupable(const QString & c) :
	QWidget(),
	state(stateIdle),
	bounce_factor(4)
{
	logic_timer = this->startTimer(1000/60);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window);
	//this->setAttribute(Qt::WA_TranslucentBackground);
	
	QPalette palette = this->palette();
		palette.setColor(this->backgroundRole(), Qt::white);
	this->setPalette(palette);


	this->load(c);
	this->show();
}

void Pickupable::load(const QString & c)
{
	if(c == "test"){
		state = states::stateIdle;
		bounce_factor = 2.0;
		this->resize(200, 200);
	}
}

void Pickupable::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::RightButton){
		QApplication::quit();
	}
	if(event->button() == Qt::LeftButton){
		state = states::stateHeld;
		held_pos = event->pos();
		velocity = QPoint(0, 0);
	}
}

void Pickupable::mouseMoveEvent(QMouseEvent * event)
{
	if(state == states::stateHeld){
		this->move(event->globalPos() - held_pos);
	}
}

void Pickupable::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton){
		state = states::stateIdle;
		velocity = QPoint(0, 0);
	}
}

void Pickupable::timerEvent(QTimerEvent * event)
{
	if(event->timerId() == logic_timer){

		QRect screen_rect = this->windowHandle()->screen()->virtualGeometry();
		QRect rect = QRect(this->pos(), this->size());

		bool on_ground = (rect.bottom() >= screen_rect.bottom());

		if(!on_ground && velocity.y() < 24) velocity.setY(velocity.y() + 1);
		else if(on_ground && velocity.y() >= 0) velocity.setY((int)(velocity.y() / -bounce_factor));

		if(state == states::stateHeld) return;

		if(!velocity.isNull()){
			this->move(this->pos() + velocity.toPoint());
		}
	}
}
