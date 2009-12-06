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

   /** Format */
   enum Attribute {
      Name       = 0x01, /// Track name
      Artist     = 0x02, /// Track artist
      Album      = 0x04, /// Track album
      Date       = 0x08, /// Last time played
      PlayCount  = 0x10  /// Play count

   };
   Q_DECLARE_FLAGS(Format, Attribute)

   /** Constructor */
   explicit Track(QGraphicsWidget *parent = 0);

   /**  Return track attribute.
     * \param key
     * \return track attribute
     */
   const QString& attrib(Attribute key) {
      return mData[key];
   }

   /** Set track attribute to given value
     * \param key
     * \param val - new value
     * \return newly assigned value
     */
   const QString& setAttribute(Attribute key, const QString& val) {
      return mData[key] = val;
   }

   /** Display format.
     * Usage:
     * %n - track name
     * %l - album
     * %a - artist
     * %d - date played
     * %p - play count
     * \return display format
     */
   const QString& format() {
      return mFormat;
   }

   /** Set display format.
     * Usage:
     * %n - track name
     * %l - album
     * %a - artist
     * %d - date played
     * %p - play count
     * \param fmt - format string
     */
   void setFormat(const QString& fmt);

   /** Return bar flags.
       \return flags
     */
   Flags flags() { return mFlags; }
   
   /** Set bar flags.
       \param flags
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

   /** Return QString representation.
     */
   QString toString();

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

   /** Update text and tooltip for widget size.
     */
   void updateLabel();
   void resizeEvent(QGraphicsSceneResizeEvent *event);
   void paint(QPainter *painter,
              const QStyleOptionGraphicsItem *option,
              QWidget *widget);

   private:  
   Flags mFlags;     /// Flags
   float mBarValue;  /// Progress 0.0f - 1.0f
   QColor mBarColor; /// Bar color (default red)
   QString mFormat;  /// Display text
   QMap<Attribute,QString> mData; /// Data map
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Track::Flags)

#endif // TRACK_H
