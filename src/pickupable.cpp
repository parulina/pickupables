#include "pickupable.h"

#include <QMovie>
#include <QWindow>
#include <QScreen>
#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QDebug>

#include <QLabel>
#include <QVBoxLayout>

Pickupable::Pickupable(const QString & c) :
	QWidget(),
	state(stateIdle),
	ditzy(0), jumpy(false),
	bounce_factor(4),

	cur_sprite(nullptr),
	sprite_mirrored(false)
{

	logic_timer = this->startTimer(1000/60);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setAttribute(Qt::WA_ShowWithoutActivating);
	
	QPalette palette = this->palette();
		palette.setColor(this->backgroundRole(), Qt::gray);
		palette.setColor(this->foregroundRole(), Qt::black);
	this->setPalette(palette);


	this->load(c);
	this->show();

	this->move(QCursor::pos());

	QMovie *movie = new QMovie("test.gif");
	connect(movie, &QMovie::updated, [=](){ this->update(); });
	movie->start();
	cur_sprite = movie;
}

void Pickupable::load(const QString & c)
{
	if(c == "test"){

		velocity = QPointF(0, 0);
		state = states::stateIdle;
		ditzy = 0;

		this->resize(200, 200);
		bounce_factor = 2.0;
		jumpy = false;

		qsrand(849218591);
		holdpos_time = ((qrand() % 200) + 100);
		qsrand(0);
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
		velocity = (event->pos() - held_pos);
	}
}

void Pickupable::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton){
		state = states::stateIdle;
		ditzy += 100;
	}
}

void Pickupable::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	const QPixmap & pix = cur_sprite->currentPixmap();

	QTransform trans = painter.transform();
	if(sprite_mirrored){
		trans.scale(-1, 1);
		trans.translate(-this->width(), 0);
	} else {
		trans.scale(1, 1);
		trans.translate(0, 0);
	}
	painter.setTransform(trans);

	QPoint pix_point = QPoint(this->width()/2 - pix.width()/2, this->height() - pix.height());
	painter.drawPixmap(QRect(pix_point, pix.size()), pix);
}

void Pickupable::timerEvent(QTimerEvent * event)
{
	if(event->timerId() == logic_timer){

		QRect screen_rect = this->windowHandle()->screen()->virtualGeometry();
		QRect rect = QRect(this->pos(), this->size());

		bool on_ground = (rect.bottom() >= screen_rect.bottom());

		if((on_ground && velocity.y() > 0) ||
		   (rect.top() <= screen_rect.top() && velocity.y() < 0))
			velocity.setY((int)(velocity.y() / -bounce_factor));

		if((rect.right() >= screen_rect.right() && velocity.x() > 0) ||
		   (rect.left() <= screen_rect.left() && velocity.x() < 0))
			velocity.setX((int)(velocity.x() / -bounce_factor));

		if(!on_ground && velocity.y() < 24) velocity.setY(velocity.y() + 1);
		if(on_ground && state == states::stateIdle && velocity.x() != 0)
			velocity.setX((int)(velocity.x() / 1.4));

		if(state == states::stateHeld) {
			velocity -= QPointF(velocity.x() /2, velocity.y() /2);
			return;
		}

		if(!velocity.isNull()){
			this->move(this->pos() + velocity.toPoint());

			sprite_mirrored = (velocity.x() < 0);

			if(qAbs(velocity.x()) > 1){
				cur_sprite->setSpeed(qFloor((qAbs(velocity.x()) / 1.0) * 100));
				cur_sprite->setPaused(false);
			} else {
				cur_sprite->setSpeed(100);
				cur_sprite->setPaused(true);
			}

			if(state != states::stateMoving && ditzy < 4000)
				ditzy += (velocity.x() + velocity.y())/2;
		} else {
			if(ditzy > 0) ditzy -= 2;
			if(ditzy <= 0) ditzy = 0;
		}

		// some other ai stuff
		if(ditzy == 0){
			QPoint cpos = QCursor::pos(),
			       bpos = (this->pos() + QPoint(this->size().width()/2, this->size().height()/2));

			if(qAbs(bpos.x() - cpos.x()) > 50){
				state = states::stateMoving;
				velocity.setX((cpos.x() > bpos.x()) ? 2 : -2);
				if(jumpy && velocity.y() == 0) velocity.setY(-15);
			} else {
				state = states::stateIdle;
				if(ditzy <= holdpos_time) ditzy += holdpos_time;
			}
		}
	}
}
