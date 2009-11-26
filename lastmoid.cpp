/***************************************************************************
 *   Copyright (C) 2008 by Damien Lévin <dml_aon@hotmail.com>      	   *
 *                 2009 by Marek Vavrusa <marek@vavrusa.com>      	   *
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

#include <QGraphicsLinearLayout>
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QHttp>
#include <QTimer>
#include <QList>
#include <Plasma/Theme>
#include <Plasma/ScrollWidget>
#include <Plasma/BusyWidget>
#include <KConfigDialog>
#include "lastmoid.h"
#include "track.h"
#include "ui_lastmoidConfig.h"
#include <QFile>
// Data type (mapped to selector)
enum Data {
   RecentTracks = 0,
   TopAlbums    = 1,
   TopArtists   = 2,
   TopTracks    = 3
};

// Data period
enum Period {
   Weekly       = 0,
   Overall      = 1,
   ThreeMonths  = 2,
   SixMonths    = 3,
   TwelveMonths = 4
};

// State tracking
enum State {
   NotFound    = 0x00, // User not found
   Finalizing  = 0x01, // Querying user
   Identified  = 0x02  // Matching user found
};

struct Lastmoid::Private
{
   Private() : interval(0), data(0), period(0), limit(0), lastDate(0), connId(-1),
               state(NotFound), layout(0), dataLayout(0), scrollWidget(0),
               busyWidget(0)
   {}

   // Config
   int interval;
   int data;
   int period;
   int limit;
   QString dataStr;
   QString periodStr;
   QString login;

   // Containers
   int lastDate;
   int connId;
   State state;
   QImage avatar;
   QHttp http;
   QTimer timer;
   Ui::lastmoidConfig configUi;
   Plasma::Svg svgLogo;

   // Widgets
   QList<Track*> trackList;
   QGraphicsLinearLayout* layout;
   QGraphicsLinearLayout* dataLayout;
   Plasma::ScrollWidget* scrollWidget;
   Plasma::BusyWidget*   busyWidget;
};

Lastmoid::Lastmoid(QObject *parent, const QVariantList &args)
   : Plasma::Applet(parent, args), d(new Private)
{
   // Init widget defaults
   d->svgLogo.setImagePath("widgets/lastmoid");
   setBackgroundHints(DefaultBackground);
   resize(220, 300);
}


Lastmoid::~Lastmoid()
{
   // Stop timer
   d->timer.stop();
   delete d;
}

void Lastmoid::init()
{
   // Prepare scroll widget
   QFontMetrics fm(font());
   d->scrollWidget = new Plasma::ScrollWidget(this);
   QGraphicsWidget* dataWidget = new QGraphicsWidget(d->scrollWidget);
   d->scrollWidget->setWidget(dataWidget);
   d->dataLayout = new QGraphicsLinearLayout(Qt::Vertical, dataWidget);
   d->busyWidget = new Plasma::BusyWidget(this);
   d->busyWidget->hide();

   // Contents widget
   d->layout = new QGraphicsLinearLayout(Qt::Vertical, this);
   d->layout->setContentsMargins(0,60 + fm.height() * 0.5,0,0);

   // Connect timer and http results
   connect(&d->timer, SIGNAL(timeout()), this, SLOT(refresh()));
   connect(&d->http, SIGNAL(requestFinished(int, bool)),this, SLOT(httpResponse(int, bool)));

   // Load config and start
   loadConfig();
   if(!d->login.isEmpty()) {
      fetch();
   }
}

void Lastmoid::refresh()
{
   // Reset timer
   d->timer.stop();
   d->timer.setInterval(d->interval * 60 * 1000);
   d->timer.start();

   // Fetch immediately
   fetch();
}

void Lastmoid::createConfigurationInterface(KConfigDialog *parent)
{
   // Create configuration widget
   QWidget *widgetConfig = new QWidget;
   KConfigGroup configGroup = config();
   d->configUi.setupUi(widgetConfig);
   d->configUi.user->setText(configGroup.readEntry("user"));
   d->configUi.dataType->setCurrentIndex(configGroup.readEntry("dataType", "0").toInt());
   d->configUi.dataPeriod->setCurrentIndex(configGroup.readEntry("dataPeriod", "1").toInt());
   d->configUi.timer->setValue(configGroup.readEntry("timer", "5").toInt());
   d->configUi.limit->setValue(configGroup.readEntry("limit", "20").toInt());

   // Connect controls
   connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
   parent->setButtons(KDialog::Ok | KDialog::Cancel);
   parent->addPage(widgetConfig, i18n("Configuration"), icon());
}

void Lastmoid::configAccepted()
{
   // Save configuration
   KConfigGroup configGroup = config();
   configGroup.writeEntry("user", d->configUi.user->text());
   configGroup.writeEntry("dataType", QString::number(d->configUi.dataType->currentIndex()));
   configGroup.writeEntry("dataPeriod", QString::number(d->configUi.dataPeriod->currentIndex()));
   configGroup.writeEntry("timer", QString::number(d->configUi.timer->value()));
   configGroup.writeEntry("limit", QString::number(d->configUi.limit->value()));
   configGroup.sync();

   // Reload configuration
   loadConfig();

   // Remove all elements
   while(!d->trackList.empty()) {
      d->dataLayout->removeAt(0);
      d->trackList.front()->deleteLater();
      d->trackList.pop_front();
   }

   // Reset last date and timers
   d->lastDate = 0;
   d->timer.stop();

   // Reset avatar and reset state
   d->avatar = QImage();
   d->state = NotFound;

   // Refresh if user available
   if(!d->login.isEmpty()) {
      d->scrollWidget->show();
      refresh();
   }
   else {
      d->scrollWidget->hide();
   }
}


void Lastmoid::loadConfig()
{
   // Translate index to QString (watch indexes in lastmoidConfig.ui)
   static QString dataTable[4] = {
      "recentTracks", "album", "artist", "track" };
   static QString periodTable[5] = {
      "7day", "overall", "3month", "6month", "12month" };

   // Load configuration data
   KConfigGroup configGroup = config();
   d->login = configGroup.readEntry("user");
   d->data = configGroup.readEntry("dataType", "0").toInt();\
   d->dataStr = dataTable[d->data % 4];
   d->period = configGroup.readEntry("dataPeriod", "0").toInt();
   d->periodStr = periodTable[d->period % 5];
   d->interval =  QString(configGroup.readEntry("timer", "5")).toInt();
   d->limit =  QString(configGroup.readEntry("limit", "20")).toInt();

   // Interval range check
   if(d->interval <= 0)
      d->interval = 5;

   // Update scene rect
   update();
}

void Lastmoid::fetch()
{
   // Is user initialised?
   QString url("http://ws.audioscrobbler.com/2.0/?method=");
   switch(d->state) {
   case Identified:

      // Recent Tracks
      if(d->data == RecentTracks) {
         url.append("user.getrecenttracks&user=" + d->login + "&limit=" + QString::number(d->limit));
      }
      else {
         // Charts
         url.append("user.gettop" + d->dataStr + "s&user=" + d->login + "&period=" + d->periodStr);
      }
      break;

   default:
      url.append("user.getinfo&user=" + d->login);
      break;
   }

   // Execute request
   httpQuery(QUrl(url + "&api_key=b6eb61f91b89e55548dd14732ee0b8a1"));
}

void Lastmoid::httpQuery(const QUrl &url)
{
   d->http.setHost(url.host());
   d->connId = d->http.get(url.toString());
   //qDebug() << "Query (" << d->connId << "): " << url.toString();
}

void Lastmoid::httpResponse(int id, bool error)
{
   // Invalid request
   if(d->connId != id) {
      return;
   }

   //qDebug() << "Response (" << id << "): received " << d->http.bytesAvailable();

   // Error checking
   if(error) {
      qWarning("received error during HTTP fetch.");
      update();
      return;
   }

   // Evaluate result
   QByteArray data (d->http.readAll());
   bool result = false;
   switch(d->state) {
   case NotFound: // Idle state
      setBusy(true);
      result = parseUserData(data);
      break;

   case Finalizing: // Awaiting user data result

      // Create avatar
      d->avatar.loadFromData(data, "JPG");
      if(!d->avatar.isNull()) {
         d->avatar = d->avatar.scaledToHeight(60);
         d->state = Identified;
         result = true;
         refresh();
         update();
      }
      break;

   case Identified: // Awaiting data
      // Parse result
      if(d->data == RecentTracks) result = parseRecentTracks(data);
      else                        result = parseStatData(data);

      // Remove busy widget
      if(result && d->busyWidget->isVisible()) {
         setBusy(false);
      }
      break;

   default:
      break;
   }
}

bool Lastmoid::parseStatData(const QByteArray& data)
{
   // Create DOM and import data
   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;
   doc.setContent(data);
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement(d->dataStr);

   // Data period element
   element = root.firstChildElement("top" + d->dataStr + "s");

   // Enter group
   element = element.firstChildElement(d->dataStr);
   if(element.isNull()) {
      return false;
   }

   // Evaluate elements
   QFontMetrics fnm(font());
   QString artist, name;
   int maxCount = element.firstChildElement("playcount").text().toInt();
   for(int i = 0; i < d->limit && !element.isNull(); element = element.nextSiblingElement(d->dataStr)) {

      // Get basic data
      artist = element.firstChildElement("artist").firstChildElement("name").text();
      name = element.firstChildElement("name").text();

      // Check track list for match
      Track* track = 0;
      if(i < d->trackList.size()) {

         // Attempt to find track
         for(int k = 0; k < d->trackList.size(); ++k) {

            // Expect track here
            track = d->trackList.at(k);

            // Compare artist - name
            if(track->attrib(Track::Artist) == artist &&
               track->attrib(Track::Name) == name) {

               // Found it, move and save
               if(i != k) {
                  d->trackList.move(k, i);
                  d->dataLayout->removeItem(track);
                  d->dataLayout->insertItem(i, track);
               }
               break;
            }
         }
      }

      // Create or reuse label
      if(track == 0) {

         // Reuse on list exceeding limit
         if(d->trackList.size() == d->limit) {
            // Take last
            track = d->trackList.back();
            d->trackList.move(d->trackList.size() - 1, 0);

         } else {
            // Create new track record
            track = new Track(this);

            // Update flags
            track->setFlags(Track::EdgeMark|Track::ElideText);
            d->trackList.insert(i, track);
         }

         // Reinsert
         d->dataLayout->insertItem(i, track);

         // Update format
         track->setAttribute(Track::PlayCount, element.firstChildElement("playcount").text());
         switch(d->data) {
         case TopAlbums:
         case TopTracks:
            track->setAttribute(Track::Artist, element.firstChildElement("artist").firstChildElement("name").text());
            track->setAttribute(Track::Name, element.firstChildElement("name").text());
            track->setFormat(" %a - %n");
            break;
         case TopArtists:
            track->setAttribute(Track::Artist, element.firstChildElement("name").text());
            track->setFormat(" %a");
            break;
         default:
            break;
         }
      }

      // Update play count
      track->animate("bar", track->barValue(), track->attrib(Track::PlayCount).toInt() / (float) maxCount);
      ++i;
   }

   return true;
}

bool Lastmoid::parseRecentTracks(const QByteArray& data)
{
   // Create DOM and import data
   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;

   doc.setContent (data);
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement("recenttracks");
   element = element.lastChildElement("track"); // Reverse read

   // Check expected element
   if(element.isNull())
      return false;

   // Get new items
   static bool lastFlip = false;
   for(;!element.isNull(); element = element.previousSiblingElement("track")) {

      // Check last date
      int uts = element.firstChildElement("date").attribute("uts").toInt();

      // Check "Now Playing"
      if(uts == 0) {
         qDebug() << "fixme: track is now playing (skipping)";
         continue;
      }

      // Check if is newer
      if(uts <= d->lastDate)
         continue;

      // Update date
      d->lastDate = uts;

      // Create label
      Track* track = 0;

      // Create or reuse
      if(d->trackList.size() < d->limit) {
         // Create track
         track = new Track(this);
         track->setFlags(Track::ElideText);
         d->trackList.push_front(track);
      }
      else {
         // Take last
         track = d->trackList.back();
         d->trackList.move(d->trackList.size() - 1, 0);
      }

      // Reinsert
      d->dataLayout->insertItem(0, track);

      // Update attributes
      track->setAttribute(Track::Name, element.firstChildElement("name").text());
      track->setAttribute(Track::Artist, element.firstChildElement("artist").text());
      track->setFormat(" %a - %n");

      // Flip bar value
      lastFlip = !lastFlip;
      track->setBarValue(1.0 * (int) lastFlip);

      // Animate insert
      if(d->trackList.size() == d->limit)
         track->animate("opacity", 0.0, 1.0);
   }

   return true;
}

bool Lastmoid::parseUserData(const QByteArray& data)
{
   // Create DOM and import data
   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;

   doc.setContent (data);
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement("user");
   element = element.firstChildElement("image");

   // Check starting element
   if(!element.isNull()) {

      // Fetch avatar
      d->state = Finalizing;
      httpQuery(QUrl(element.text()));
      return true;
   }

   return false;
}

void Lastmoid::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
   // Applet paint
   Plasma::Applet::paintInterface(p, option, contentsRect);

   // Measures
   QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
   QFontMetrics fm(font);
   int widgetWidth = (int)contentsRect.width();
   int hAlign = fm.height();

   // SVG logo
   p->setRenderHint(QPainter::SmoothPixmapTransform);
   p->setRenderHint(QPainter::Antialiasing);
   d->svgLogo.resize(94, 48);
   d->svgLogo.paint(p,hAlign, (int)contentsRect.top());


   p->save();

   // User string
   if(!d->login.isEmpty()) {
      QPoint headerPt(contentsRect.topLeft());
      headerPt.setX(hAlign);
      headerPt.setY(headerPt.y() + d->svgLogo.size().height() + 2);
      p->setPen(QColor(213,13,6));
      p->drawText(headerPt, d->login);
   } else {
      // Placeholder
      QRect msgRect(contentsRect);
      msgRect.setTop(msgRect.top() + d->svgLogo.size().height() + 2);
      msgRect.adjust(hAlign,hAlign,-hAlign,-hAlign);
      msgRect.setHeight(msgRect.height() * 0.5);

      // Icon
      KIcon icon("configure");
      p->setOpacity(0.8);
      icon.paint(p, msgRect, Qt::AlignTop|Qt::AlignHCenter);
      p->setOpacity(1.0);

      // Text
      msgRect.moveTop(msgRect.top() + msgRect.height());
      p->drawText(msgRect, Qt::AlignCenter, tr("User not set."));

   }

   // Avatar
   if(!d->avatar.isNull()) {

      // Create clip path
      QPainterPath avClip;
      QRect avFrame((int)contentsRect.left() + widgetWidth - 60, (int)contentsRect.top(), 60, 60);
      avClip.addRoundedRect(QRectF(avFrame), 10.0, 10.0);

      // Draw clipped avatar
      p->setClipPath(avClip);
      if(!d->avatar.isNull())
         p->drawImage(avFrame, d->avatar);
      p->setClipping(false);

      // Avatar border
      QPen pen(p->pen());
      pen.setColor(Qt::lightGray);
      pen.setWidth(2);
      p->setPen(pen);
      p->drawRoundedRect(avFrame, 10.0, 10.0);
   }

   p->restore();
}

void Lastmoid::setBusy(bool val)
{
   // Remove current widget
   if(d->layout->count())
      d->layout->removeAt(0);

   // If true, replace widget with "Busy" indicator
   if(val) {
      d->layout->addItem(d->busyWidget);
      d->busyWidget->show();
      d->scrollWidget->hide();
   }
   else {
      d->layout->addItem(d->scrollWidget);
      d->busyWidget->hide();
      d->scrollWidget->show();
   }
}

#include "lastmoid.moc"
