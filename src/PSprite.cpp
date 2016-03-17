#include "PSprite.h"

#include <QTimer>
#include <QDebug>

spriteFrameInfo::spriteFrameInfo(const QImage & img, int delay) :
	image(img), image_delay(delay)
{
}

PSprite::PSprite(QImageReader & reader, QObject * parent) :
	QObject(parent),
	current_frame(0),
	loop_count(0), looped_count(0),
	animation_speed(100), animation_paused(false),
	sprite_timer(new QTimer(this))
{
	sprite_timer->setSingleShot(true);
	connect(sprite_timer, &QTimer::timeout, this, &PSprite::timerSlot);

	loop_count = reader.loopCount();
	while(reader.canRead()){
		QImage img = reader.read();
		int delay = reader.nextImageDelay();
		frames.push_back(spriteFrameInfo(img, delay));
	}
	if(!frames.isEmpty()){
		frame_palette = frames.first().image.colorTable();
	}
	this->setCurrentFrame(0);
	sprite_timer->stop();
}

bool PSprite::setCurrentFrame(int frame)
{
	if(frames.isEmpty()) return false;
	if(frame < 0 || frame >= frames.size()) return false;

	const spriteFrameInfo & frameinfo = frames.at((current_frame = frame));

	QImage mod_img = frameinfo.image;
		mod_img.setColorTable(frame_palette);
	current_pixmap = QPixmap::fromImage(frameinfo.image);

	if(frames.size() && !animation_paused){
		sprite_timer->start(frameinfo.image_delay * (animation_speed / 100.0));
	}
	emit updated();
	return true;
}

void PSprite::timerSlot()
{
	if(frames.isEmpty()) {
		sprite_timer->stop();
		return;
	}
	if(frames.size()){
		current_frame ++;
		if(current_frame == frames.size()){
			if(loop_count != -1 && looped_count >= loop_count){
				sprite_timer->stop();
				return;
			}
			looped_count ++;
			current_frame = 0;
		}
		this->setCurrentFrame(current_frame);
	}
}

QVector<QRgb> PSprite::palette()
{
	return frame_palette;
}

void PSprite::setPalette(QVector<QRgb> & palette)
{
	frame_palette = palette;
}

void PSprite::setPaletteSlot(int index, QRgb color)
{
	if(index < 0 || index >= frame_palette.size()) return;
	frame_palette[index] = color;
}

void PSprite::setSpeed(int s)
{
	animation_speed = s;
}

int PSprite::speed()
{
	return animation_speed;
}

void PSprite::setPaused(bool paused)
{
	animation_paused = paused;
	this->setCurrentFrame(current_frame);
}

QPixmap PSprite::currentPixmap()
{
	return current_pixmap;
}
