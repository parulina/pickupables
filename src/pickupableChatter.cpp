#include "pickupableChatter.h"

#include <QResizeEvent>
#include <QVBoxLayout>
#include <QDebug>

#include "pickupableChatBubble.h"

PickupableChatter::PickupableChatter(QWidget * parent) :
	QWidget(parent)
{
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
	this->setAttribute(Qt::WA_ShowWithoutActivating);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setAttribute(Qt::WA_TransparentForMouseEvents);
	
	this->setLayout((layout = new QVBoxLayout));
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->addStretch(1);

	this->setFixedSize(100, 50);
	this->show();
}

void PickupableChatter::resizeEvent(QResizeEvent * event)
{
	this->move(this->x(), this->y() - (event->size() - event->oldSize()).height());
}

void PickupableChatter::chat(const QString & message)
{
	PickupableChatBubble * label = new PickupableChatBubble(message, this);
	layout->addWidget(label, 0, Qt::AlignBottom);
}
