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

#include <QGraphicsWidget>
#include <QPainter>
#include "barlabel.h"

BarLabel::BarLabel(QGraphicsWidget *parent) :
    Plasma::Label(parent), mBarColor(QColor(215, 0, 25, 96)),
    mProgress(0.0)
{
}

float BarLabel::setBar(float val)
{
   float oldVal = mProgress;
   mProgress = val;
   update(boundingRect());
   return oldVal;
}

QColor BarLabel::setColor(const QColor& color)
{
   QColor oldColor = mBarColor;
   mBarColor = color;
   update(boundingRect());
   return oldColor;
}

void BarLabel::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
   // Paint background
   if(mProgress > 0.0) {
      QRectF barRect = boundingRect();
      barRect.setWidth(barRect.width() * mProgress);
      painter->fillRect(barRect, QBrush(mBarColor));
   }

   // Paint widget
   Plasma::Label::paint(painter, option, widget);
}

#include "barlabel.moc"
