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

#ifndef BARLABEL_H
#define BARLABEL_H

#include <Plasma/Label>
class QGraphicsWidget;

class BarLabel : public Plasma::Label
{
   Q_OBJECT
   public:
   explicit BarLabel(QGraphicsWidget *parent = 0);
   float setBar(float bar);
   QColor setColor(const QColor& color);

   protected:
   void paint(QPainter *painter,
              const QStyleOptionGraphicsItem *option,
              QWidget *widget);

   private:
   QColor mBarColor;
   float mProgress;
};

#endif // BARLABEL_H
