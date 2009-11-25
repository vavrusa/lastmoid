/***************************************************************************
 *   Copyright (C) 2009 by Marek Vavrusa <marek@vavrusa.com>      	   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <Plasma/Animation>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsWidget>
#include <QFontMetrics>
#include <QPainter>
#include "barlabel.h"

BarLabel::BarLabel(QGraphicsWidget *parent) :
    Plasma::Label(parent), mBarColor(QColor(215, 0, 25, 96)),
    mBarValue(0.0), mBarFlags(NoFlags)
{
   setScaledContents(false);
}

void BarLabel::setBarFlags(int flags)
{
   mBarFlags = flags;
}

void BarLabel::animate(const QByteArray& property, const QVariant& from, const QVariant& to)
{
   QPropertyAnimation* anim = new QPropertyAnimation(this, property);
   anim->setDuration(500);
   anim->setStartValue(from);
   anim->setEndValue(to);
   anim->start(QPropertyAnimation::DeleteWhenStopped);
}

float BarLabel::setBarValue(float val)
{
   float oldVal = mBarValue;
   mBarValue = val;
   update(boundingRect());

   return oldVal;
}

QColor BarLabel::setBarColor(const QColor& color)
{
   QColor oldColor = mBarColor;
   mBarColor = color;
   update(boundingRect());
   return oldColor;
}

void BarLabel::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   Plasma::Label::resizeEvent(event);

   if(barFlags() & ElideText) {
      QFontMetrics fnm(font());
      QString text = fnm.elidedText(name(), Qt::ElideRight, event->newSize().width());
      setText(text);
   }
}

void BarLabel::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
   // Paint background
   if(mBarValue > 0.0) {
      painter->setRenderHint(QPainter::Antialiasing);

      QRectF barRect = boundingRect();
      barRect.setWidth(barRect.width() * mBarValue);

      // Bar color
      QColor barColor(mBarColor);
      if(barFlags() & NowPlaying)
         barColor = QColor(255, 252, 202, 96); // Soft yellow

      // Edge decoration for progress
      if(barFlags() & EdgeMark) {
         float edgeLen = barRect.height() * 0.5;
         barRect.setWidth(barRect.width()  - edgeLen);
         QPainterPath path(barRect.topLeft());
         path.lineTo(barRect.topRight());
         path.lineTo(barRect.topRight() + QPointF(edgeLen, barRect.height() * 0.5));
         path.lineTo(barRect.bottomRight());
         path.lineTo(barRect.bottomLeft());
         path.closeSubpath();
         painter->fillPath(path, QBrush(barColor));
      }
      else {
         painter->fillRect(barRect, QBrush(barColor));
      }
   }

   // Paint widget
   Plasma::Label::paint(painter, option, widget);
}

#include "barlabel.moc"
