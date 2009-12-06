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


#include <QtGlobal>
#include <Plasma/Animator>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsWidget>
#include <QFontMetrics>
#include <QPainter>
#include "track.h"

Track::Track(QGraphicsWidget *parent) :
    Plasma::Label(parent), mBarValue(0.0), mBarColor(QColor(215, 0, 25, 96))
{
   // Set default size
   QFontMetrics fnm(font());
   setScaledContents(false);
   setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
   setMaximumHeight(fnm.height() * 1.25);
}

void Track::setFlags(Flags flags)
{
   mFlags = flags;
   updateLabel();
}

void Track::setFormat(const QString &fmt)
{
   mFormat = fmt;
   updateLabel();
}

float Track::setBarValue(float val)
{
    float oldVal = mBarValue;
    mBarValue = val;
    update();

    return oldVal;
}

QColor Track::setBarColor(const QColor& color)
{
   QColor oldColor = mBarColor;
   mBarColor = color;
   update();
   return oldColor;
}

void Track::updateLabel()
{
   // Get text
   QString text = toString();

   // Update toolTip
   if(!attrib(PlayCount).isEmpty())
    setToolTip(text + tr(" (%1 plays)").arg(attrib(PlayCount)));
   else
    setToolTip(text);

   // Elide text
   if(flags() & ElideText) {
      QFontMetrics fnm(font());
      setText(fnm.elidedText(text, Qt::ElideRight, contentsRect().width()));
   }
}

QString Track::toString() {
   QString res(format());
   res.replace("%n", attrib(Name));
   res.replace("%l", attrib(Album));
   res.replace("%a", attrib(Artist));   
   res.replace("%d", attrib(Date));
   res.replace("%p", attrib(PlayCount));
   return res;
}

void Track::resizeEvent(QGraphicsSceneResizeEvent *event)
{
   Plasma::Label::resizeEvent(event);

   // Update label
   updateLabel();
}

void Track::paint(QPainter *painter,
           const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
   // Unused
   Q_UNUSED(option);
   Q_UNUSED(widget);

   // Paint background
   if(mBarValue > 0.0) {
      painter->setRenderHint(QPainter::Antialiasing);

      QRectF barRect = boundingRect();
      barRect.setWidth(barRect.width() * mBarValue);

      // Bar color
      QColor barColor(mBarColor);
      if(flags() & NowPlaying)
         barColor = QColor(255, 252, 202, 96); // Soft yellow

      // Edge decoration for progress
      if(flags() & EdgeMark) {

         // Avoid overflow
         float edgeLen = barRect.height() * 0.5;
         if(barRect.width() > edgeLen)
            barRect.setWidth(barRect.width()  - edgeLen);
         else
            barRect.setWidth(0.0);

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

   // Paint text
   painter->drawText(boundingRect(), text());
}

#include "track.moc"
