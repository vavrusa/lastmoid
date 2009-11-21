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
   Q_PROPERTY(int textFlags READ textFlags WRITE setTextFlags)
   Q_PROPERTY(float bar READ barValue WRITE setBarValue)
   Q_PROPERTY(QColor barColor READ barColor WRITE setBarColor)

   public:
   explicit BarLabel(QGraphicsWidget *parent = 0);

   /** Overloaded for storing original text.
       Stored text used for elided.
       \param text text string
     */
   void setText(const QString& text) {
      Plasma::Label::setText(mText = text);
   }

   /** Return bitmask of text flags defined in BarLabel::Flags
       \return bitmask of flags
     */
   int textFlags() { return mTextFlags; }
   /** Set text flags defined in BarLabel::Flags
       \param flags bitmask of flags
     */
   void setTextFlags(int flags) { mTextFlags = flags; }

   /** Return bar percentage 0.0 - 1.0f;
       \param bar percentage
     */
   float barValue() { return mBarValue; }
   /** Set bar percentage to given value
       \param bar float value 0.0f - 1.0f
       \return previous value
     */
   float setBarValue(float bar);

   /** Current bar color
       \return bar color
     */
   QColor barColor() { return mBarColor; }

   /** Set bar color to given value
       \param color new color
       \return previous color
     */
   QColor setBarColor(const QColor& color);

   /** Text flags */
   enum TextFlags {
      NoFlags   = 0x00,
      ElideText = 0x01,
      EdgeMark  = 0x02
   };

   protected:
   void resizeEvent(QGraphicsSceneResizeEvent *event);
   void paint(QPainter *painter,
              const QStyleOptionGraphicsItem *option,
              QWidget *widget);

   private:
   QString mText;   // Original text
   QColor mBarColor;// Bar color (default red)
   float mBarValue; // Progress 0.0f - 1.0f
   int mTextFlags;  // Text flags
};

#endif // BARLABEL_H
