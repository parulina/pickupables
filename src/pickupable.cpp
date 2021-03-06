#include "pickupable.h"

#include <QMovie>
#include <QWindow>
#include <QScreen>
#include <QApplication>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QDebug>

#include <QVBoxLayout>

#include "src/pickupableWindowList.h"
#include "src/pickupableChatter.h"
#include "src/PSprite.h"

#include "karchive/KZip"

QList<windowInfo> window_list;


Pickupable::Pickupable(const QString & c) :
	QWidget(),

	velocity(0, 0), cur_direction(directionRight), cur_state(stateIdle), on_ground(false),
	cur_sprite(nullptr),

	held_pos(0, 0), ditzy(0), idle_timeout(10), jumpy(false),

	bounce_factor(4), holdpos_time(0), idle_level(stateMax-1)
{
	chatter = new PickupableChatter;

	logic_timer = this->startTimer(1000/60);
	idle_timer = this->startTimer(1000/4);
	windowlist_timer = this->startTimer(500);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
	this->setAttribute(Qt::WA_ShowWithoutActivating);
	this->setAttribute(Qt::WA_TranslucentBackground);

	this->resize(100, 100);
	this->load(c);
	this->show();

	this->move(QCursor::pos());
}

void Pickupable::reset()
{
	velocity = QPointF(0, 0);
	qDeleteAll(sprites); sprites.clear();
	this->setState(states::stateIdle);
	cur_sprite = nullptr;
	ditzy = 0;
}

typedef std::uniform_int_distribution<int> randint;

bool Pickupable::load(const QString & c)
{
	KZip zip(c);
	if(!zip.open(QIODevice::ReadOnly)) return false;

	const KArchiveDirectory * dir = zip.directory();
	if(!dir) return false;

	qDebug() << "Zip" << c << "contents:" << dir->entries();
	const KArchiveFile * info_file = static_cast<const KArchiveFile*>(dir->entry("info.json"));
	if(info_file && info_file->isFile()){
		QJsonParseError err;
		QJsonDocument doc = QJsonDocument::fromJson(info_file->data(), &err);
		if(err.error == QJsonParseError::NoError || doc.isObject()){
			const QJsonObject info_obj = doc.object();

			if(info_obj.contains("width") && info_obj.value("width").isDouble())
				this->setFixedWidth(info_obj.value("width").toInt(50));

			if(info_obj.contains("height") && info_obj.value("height").isDouble())
				this->setFixedHeight(info_obj.value("height").toInt(50));

			if(info_obj.contains("bounce") && info_obj.value("bounce").isDouble())
				this->bounce_factor = info_obj.value("bounce").toDouble(2.0);

			if(info_obj.contains("jumpy") && info_obj.value("jumpy").isBool())
				this->jumpy = info_obj.value("jumpy").toBool(false);

			if(info_obj.contains("idle") && info_obj.value("idle").isDouble())
				this->idle_level = info_obj.value("idle").toInt(stateMax-1);

			if(info_obj.contains("seed") && info_obj.value("seed").isDouble()) {

				std::mt19937 mt(info_obj.value("seed").toInt(0));
				holdpos_time = randint(100, 300)(mt);

				qDebug() << "holdpos_time:" << holdpos_time;
			}
		}
	}

	foreach(const QString & filename, dir->entries()){
		if(!filename.endsWith(".gif")) continue;

		const KArchiveEntry * entry = static_cast<const KArchiveEntry *>(dir->entry(filename));
		if(!entry || !entry->isFile()) continue;

		const KZipFileEntry * file = static_cast<const KZipFileEntry*>(entry);
		if(!file) continue;

		const QString & basename = file->name().section('.', 0, 0);

		QIODevice * dev = file->createDevice();
		if(dev){

			QImageReader ir(dev);
			sprites[basename] = new PSprite(ir, this);
			connect(sprites[basename], &PSprite::updated, this, static_cast<void(Pickupable::*)(void)>(&Pickupable::update));

			qDebug() << "Adding" << basename;
			delete dev;
		}
	}

	this->setState(states::stateIdle);
	return true;
}

Pickupable::states Pickupable::state() const
{
	return cur_state;
}

const QString Pickupable::stateName() const
{
	switch(this->state()){
		case states::stateIdle: return "idle";
		case states::stateHeld: return "held";
		case states::stateMove: return "move";
		default: return "NULL";
	}
}

void Pickupable::setState(states s)
{
	cur_state = s;

	if(sprites.contains(this->stateName())) {
		PSprite * new_anim = sprites[this->stateName()];
		bool diff_anim = (cur_sprite != new_anim);

		if(cur_sprite && diff_anim) {
			cur_sprite->setPaused(true);
			cur_sprite = nullptr;
		}
		cur_sprite = new_anim;
		if(diff_anim) {
			cur_sprite->setPaused(false);
		}
	}
}

Pickupable::directions Pickupable::direction() const
{
	return cur_direction;
}
void Pickupable::setDirection(directions d)
{
	cur_direction = d;
	this->update();
}

void Pickupable::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::MiddleButton){
		QApplication::quit();
	}
	if(event->button() == Qt::LeftButton){
		this->setState(states::stateHeld);
		held_pos = event->pos();
		velocity = QPoint(0, 0);
		chatter->chat("abababab");
	}
}

void Pickupable::mouseMoveEvent(QMouseEvent * event)
{
	if(this->state() == states::stateHeld){
		this->move(event->globalPos() - held_pos);
		velocity = (event->pos() - held_pos);
	}
}

void Pickupable::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton){
		this->setState(states::stateIdle);
		ditzy += 100;
	}
}

void Pickupable::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	if(cur_sprite){
		const QPixmap & pix = cur_sprite->currentPixmap();

		QTransform trans = painter.transform();

		if(this->direction() == directions::directionLeft){
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
}

void Pickupable::moveEvent(QMoveEvent * event)
{
	chatter->move(this->x() + this->width(), this->y() - chatter->height());
}

void Pickupable::timerEvent(QTimerEvent * event)
{
	if(event->timerId() == windowlist_timer){
		window_list = PickupableWindowList::windowGeometryList();
	}
	if(event->timerId() == idle_timer){

		if(this->state() == states::stateHeld) return;
		this->idleStateUpdate();
	}
	if(event->timerId() == logic_timer){

		const QRect rect = QRect(this->pos(), this->size());
		const QRect col_rect = rect.adjusted(0, (rect.height()-20), 0, 0);

		on_ground = 0;

		foreach(const windowInfo win, window_list){
			if(win.maximized) continue;

			const QRect tb_rect = win.rect.adjusted(0, 0, 0, -(win.rect.height()-20));
			if(col_rect.intersects(tb_rect)){
				on_ground = 2;
				break;
			}
		}
		QRect screen_rect = this->windowHandle()->screen()->virtualGeometry();

		if(!on_ground) on_ground = (int)(rect.bottom() >= screen_rect.bottom());

		if((on_ground && velocity.y() > 0) ||
		   (rect.top() <= screen_rect.top() && velocity.y() < 0))
			velocity.setY((int)(velocity.y() / -bounce_factor));

		if((rect.right() >= screen_rect.right() && velocity.x() > 0) ||
		   (rect.left() <= screen_rect.left() && velocity.x() < 0))
			velocity.setX((int)(velocity.x() / -bounce_factor));


		if(!on_ground && velocity.y() < 24) velocity.setY(velocity.y() + 1);
		if(on_ground && this->state() == states::stateIdle && velocity.x() != 0)
			velocity.setX((int)(velocity.x() / 1.4));

		if(this->state() == states::stateHeld) {
			velocity -= QPointF(velocity.x() /2, velocity.y() /2);
			return;
		}

		if(!velocity.isNull()){
			this->move(this->pos() + velocity.toPoint());

			if(this->state() == states::stateMove){
				this->setDirection((velocity.x() < 0) ? directionLeft : directionRight);
			}

			if(cur_sprite){
				if(qAbs(velocity.x()) > 1){
					//cur_sprite->setSpeed(qFloor((qAbs(velocity.x()) / 1.0) * 100));
				} else {
					cur_sprite->setSpeed(100);
				}
			}

			if(this->state() != states::stateMove && ditzy < 4000)
				ditzy += (velocity.x() + velocity.y())/2;
		} else {
			if(ditzy > 0) ditzy -= 2;
			if(ditzy <= 0) ditzy = 0;
		}
	}
}

void Pickupable::idleStateUpdate()
{
	switch(cur_idlestate){
		case stateJumping:
		{
			if(!ditzy && velocity.y() == 0) {
				velocity.setY(jumpy ? -18 : -4);
				ditzy = 100;
			}
			break;
		}
		case stateChasingPointer:
		{
			QPoint cpos = QCursor::pos(),
			       bpos = (this->pos() + QPoint(this->size().width()/2, this->size().height()/2));

			if(qAbs(bpos.x() - cpos.x()) > 50 && !ditzy){
				this->setState(states::stateMove);
				velocity.setX((cpos.x() > bpos.x()) ? 2 : -2);
				if(!ditzy && jumpy && velocity.y() == 0) velocity.setY(-15);
			} else {
				this->setDirection((bpos.x() > cpos.x()) ? directions::directionLeft : directions::directionRight);
				this->setState(states::stateIdle);
			}
			break;
		}
		case stateWalkingAround:
		{
			int x = this->x() + this->width()/2,
			    screen_left = this->windowHandle()->screen()->geometry().left() + 200,
			    screen_right = this->windowHandle()->screen()->geometry().right() - 200;

			this->setState(states::stateMove);
			velocity.setX(this->direction() == directionLeft ? -2 : 2);
			if(jumpy && velocity.y() == 0) velocity.setY(-10);

			if((x < screen_left && this->direction() == directionLeft) || (x > screen_right && this->direction() == directionRight)){
				this->setDirection((this->direction() == directionRight) ? directionLeft : directionRight);
				this->setState(states::stateIdle);
				velocity = QPointF(0, 0);
				idle_timeout = 0;
			}
			break;
		}

		// fall-through
		case stateSitting:
		{
			if(sprites.contains("sitting"))
				cur_sprite = sprites["sitting"];
		}
		case stateSleeping:
		{
			if(sprites.contains("sleeping"))
				cur_sprite = sprites["sleeping"];
		}
		case stateStill: break;
		default: break;
	}

	if(idle_timeout > 0) idle_timeout--;
	if(idle_timeout == 0 && this->state() == states::stateIdle){

		std::mt19937 mt(this->pos().x() + this->pos().y() + QTime::currentTime().second());
		cur_idlestate = static_cast<idleStates>(randint(0, idle_level)(mt));

		switch(cur_idlestate){
			case stateStill: { chatter->chat("i'm standing still now!"); break; }
			case stateSitting: { chatter->chat("i'm sitting now!"); break; }
			case stateSleeping: { chatter->chat("i'm sleeping long now!"); break; }
			case stateWalkingAround: { chatter->chat("i'm walking around now!"); break; }
			case stateJumping: { chatter->chat("i'm jumping now!"); break; }
			case stateChasingPointer: { chatter->chat("i'm chasing your pointer now!"); break; }
			default: break;
		}
		switch(cur_idlestate){
			case stateJumping:
				{ idle_timeout = randint(4, 10)(mt); break; }
			case stateSitting:
			case stateStill:
				{ idle_timeout = randint(20, 40)(mt); break; }
			case stateChasingPointer:
			case stateWalkingAround:
			case stateSleeping:
				{ idle_timeout = randint(60, 120)(mt); break; }

			default:
				idle_timeout = randint(20, 30)(mt);
		}
	}
}
