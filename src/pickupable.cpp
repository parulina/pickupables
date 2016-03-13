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

#include <QLabel>
#include <QVBoxLayout>

#include "karchive/KZip"

Pickupable::Pickupable(const QString & c) :
	QWidget(),
	cur_state(stateIdle),
	ditzy(0), jumpy(false),
	bounce_factor(4),

	cur_sprite(nullptr),
	sprite_mirrored(false)
{

	logic_timer = this->startTimer(1000/60);

	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
	this->setAttribute(Qt::WA_ShowWithoutActivating);
	
	QPalette palette = this->palette();
		palette.setColor(this->backgroundRole(), Qt::gray);
		palette.setColor(this->foregroundRole(), Qt::black);
	this->setPalette(palette);

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

			if(info_obj.contains("size") && info_obj.value("size").isDouble()){
				int size = info_obj.value("size").toInt(50);
				this->resize(size, size);
				qDebug() << "dimensions:" << size;
			}

			if(info_obj.contains("bounce") && info_obj.value("bounce").isDouble())
				this->bounce_factor = info_obj.value("bounce").toDouble(2.0);

			if(info_obj.contains("jumpy") && info_obj.value("jumpy").isBool())
				this->jumpy = info_obj.value("jumpy").toBool(false);

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
			QMovie * mov = sprites[basename] = new QMovie(dev, QByteArray(), this);
			connect(mov, &QMovie::updated, this, &Pickupable::movieUpdate);
			mov->setCacheMode(QMovie::CacheAll);

			// preload all of the frames
			mov->jumpToFrame(0);
			do {
				mov->jumpToNextFrame();
			} while(mov->currentFrameNumber() != 0);

			// for some reason deleting the device prevent the movie from starting...
			// so just put dev as child to the movie
			// so it gets deleted when the movie gets deleted... is this safe?
			dev->setParent(mov);
			mov->start();

			qDebug() << "Adding" << basename << sprites[basename] << mov->frameCount();
		}
	}

	this->setAttribute(Qt::WA_TranslucentBackground);
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
	cur_sprite = nullptr;

	if(sprites.contains(this->stateName())) {
		cur_sprite = sprites[this->stateName()];
	}
}

void Pickupable::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::RightButton){
		QApplication::quit();
	}
	if(event->button() == Qt::LeftButton){
		this->setState(states::stateHeld);
		held_pos = event->pos();
		velocity = QPoint(0, 0);
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

void Pickupable::movieUpdate(const QRect & rect)
{
	this->update();
}

void Pickupable::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	if(cur_sprite){
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
		if(on_ground && this->state() == states::stateIdle && velocity.x() != 0)
			velocity.setX((int)(velocity.x() / 1.4));

		if(this->state() == states::stateHeld) {
			velocity -= QPointF(velocity.x() /2, velocity.y() /2);
			return;
		}

		if(!velocity.isNull()){
			this->move(this->pos() + velocity.toPoint());

			sprite_mirrored = (velocity.x() < 0);

			if(cur_sprite){
				if(qAbs(velocity.x()) > 1){
					cur_sprite->setSpeed(qFloor((qAbs(velocity.x()) / 1.0) * 100));
					cur_sprite->setPaused(false);
				} else {
					cur_sprite->setSpeed(100);
					cur_sprite->setPaused(true);
				}
			}

			if(this->state() != states::stateMove && ditzy < 4000)
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
				this->setState(states::stateMove);
				velocity.setX((cpos.x() > bpos.x()) ? 2 : -2);
				if(jumpy && velocity.y() == 0) velocity.setY(-15);
			} else {
				this->setState(states::stateIdle);
				if(ditzy <= holdpos_time) ditzy += holdpos_time;
			}
		}
	}
}
