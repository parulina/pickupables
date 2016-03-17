#ifndef PSPRITE_H
#define PSPRITE_H

#include <QObject>
#include <QImageReader>
#include <QPixmap>

struct spriteFrameInfo {
	QImage image;
	int image_delay;
	spriteFrameInfo(const QImage & img, int delay);
};

class PSprite : public QObject
{
Q_OBJECT
	private:
	int current_frame;
	QPixmap current_pixmap;
	QVector<QRgb> frame_palette;
	QList<spriteFrameInfo> frames;
	int loop_count, looped_count;
	int animation_speed;
	bool animation_paused;

	QTimer * sprite_timer;
	void timerSlot();

	bool setCurrentFrame(int frame);

	public:
	PSprite(QImageReader & reader, QObject * = nullptr);

	QVector<QRgb> palette();
	void setPalette(QVector<QRgb> & palette);
	void setPaletteSlot(int index, QRgb color);

	void setSpeed(int s);
	int speed();

	void setPaused(bool paused);

	QPixmap currentPixmap();

	signals:
	void updated();
};

#endif
