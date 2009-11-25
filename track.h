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

#ifndef TRACK_H
#define TRACK_H

#include <Plasma/Label>
#include <QFlags>
class QGraphicsWidget;

/** Links track data with interactive label.
  */
class Track : public Plasma::Label
{
   Q_OBJECT
   Q_PROPERTY(QString name READ name WRITE setName)
   Q_PROPERTY(float bar READ barValue WRITE setBarValue)
   Q_PROPERTY(QColor barColor READ barColor WRITE setBarColor)

   public:

   /** Flags */
   enum Flag {
      ElideText  = 0x01,
      EdgeMark   = 0x02,
      NowPlaying = 0x04
   };
   Q_DECLARE_FLAGS(Flags, Flag)

   /** Constructor */
   explicit Track(QGraphicsWidget *parent = 0);

   /** Track name.
     * \return track name
     */
   const QString& name() {
      return mName;
   }

   /** Set track name.
       Stored text used for elided.
       \param text text string
     */
   void setName(const QString& text) {
      setText(mName = text);
   }

   /** Return bitmask of track flags defined in BarLabel::Flags
       \return bitmask of flags
     */
   Flags flags() { return mFlags; }
   
   /** Set track flags defined in BarLabel::Flags
       \param flags bitmask of flags
     */
   void setFlags(Flags flags);

   /** Return bar percentage 0.0 - 1.0f;
       \param bar percentage
     */
   float barValue() { return mBarValue; }
   /** Set bar percentage to given value
       \param bar float value 0.0f - 1.0f
       \return previous value
     */
   float setBarValue(float bar);

   /** Animate given property.
     */
   void animate(const QByteArray& property, const QVariant& from, const QVariant& to);

   /** Current bar color
       \return bar color
     */
   QColor barColor() { return mBarColor; }

   /** Set bar color to given value
       \param color new color
       \return previous color
     */
   QColor setBarColor(const QColor& color);

   protected:
   void resizeEvent(QGraphicsSceneResizeEvent *event);
   void paint(QPainter *painter,
              const QStyleOptionGraphicsItem *option,
              QWidget *widget);

   private:
   QString mName;   // Original text
   QColor mBarColor;// Bar color (default red)
   float mBarValue; // Progress 0.0f - 1.0f
   Flags mFlags; // Flags
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Track::Flags)

#endif // TRACK_H
