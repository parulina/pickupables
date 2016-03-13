#include "pickupableChatBubble.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

PickupableChatBubble::PickupableChatBubble(const QString & text, QWidget * parent) :
	QLabel(text, parent)
{
	this->setAutoFillBackground(true);
	this->setContentsMargins(6, 4, 6, 4);
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	this->setStyleSheet("QLabel { background:#AAA; color:#000; border-radius:6px; border-bottom-left-radius:0; }");

	QGraphicsOpacityEffect * effect = new QGraphicsOpacityEffect(this);
	this->setGraphicsEffect(effect);

	QPropertyAnimation * animation = new QPropertyAnimation(effect, "opacity");
	animation->setDuration(4000);
	animation->setKeyValueAt(0.0, 1);
	animation->setKeyValueAt(0.8, 1);
	animation->setKeyValueAt(1.0, 0);
	animation->start();

	connect(animation, &QPropertyAnimation::finished, this, &QLabel::deleteLater);
}
