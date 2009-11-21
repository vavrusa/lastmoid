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

#include <Plasma/ScrollWidget>
#include <Plasma/BusyWidget>
#include <QGraphicsLinearLayout>
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <KConfigDialog>
#include "lastmoid.h"
#include "barlabel.h"

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
   Private() : interval(0), data(0), period(0), state(NotFound),
               layout(0), scrollWidget(0), dataWidget(0),
               dataLayout(0), busyWidget(0)
   {}

   // Config
   int interval;
   int data;
   int period;
   QString dataStr;
   QString periodStr;
   QString login;

   // Containers
   State state;
   QImage avatar;
   QUrl url;
   QHttp http;
   QBuffer buffer;
   QTimer timer;
   Ui::lastmoidConfig configUi;
   Plasma::Svg svgLogo;
   KConfigGroup configGroup;

   // Widgets
   QGraphicsLinearLayout* layout;
   Plasma::ScrollWidget* scrollWidget;
   QGraphicsWidget*      dataWidget;
   QGraphicsLinearLayout* dataLayout;
   Plasma::BusyWidget*    busyWidget;
};

Lastmoid::Lastmoid(QObject *parent, const QVariantList &args)
   : Plasma::Applet(parent, args), d(new Private)
{
   d->svgLogo.setImagePath("widgets/lastmoid");
   setBackgroundHints(DefaultBackground);
   resize(220, 300);
}


Lastmoid::~Lastmoid()
{
   delete d;
   d->timer.stop();
}

void Lastmoid::init()
{
   // Prepare scroll widget
   QFontMetrics fm(font());
   d->scrollWidget = new Plasma::ScrollWidget(this);
   d->dataWidget = new QGraphicsWidget(d->scrollWidget);
   d->scrollWidget->setWidget(d->dataWidget);
   d->dataLayout = new QGraphicsLinearLayout(Qt::Vertical, d->dataWidget);
   d->busyWidget = new Plasma::BusyWidget(this);

   // Contents widget
   d->layout = new QGraphicsLinearLayout(Qt::Vertical, this);
   d->layout->setContentsMargins(0,60 + fm.height() * 0.5,0,0);
   d->layout->addItem(d->busyWidget);

   connect(&d->timer, SIGNAL(timeout()), this, SLOT(refresh()));
   connect(&d->http, SIGNAL(requestFinished(int, bool)),this, SLOT(httpResponse(int, bool)));

   loadConfig();
   fetch();
}


void Lastmoid::refresh()
{
   d->timer.stop();
   d->timer.setInterval(d->interval * 60 * 1000);
   d->timer.start();
   fetch();
}


void Lastmoid::createConfigurationInterface(KConfigDialog *parent)
{

   QWidget *widgetConfig = new QWidget;
   d->configGroup = config();
   d->configUi.setupUi(widgetConfig);
   d->configUi.user->setText(d->configGroup.readEntry("user"));
   d->configUi.dataType->setCurrentIndex(d->configGroup.readEntry("dataType", "0").toInt());
   d->configUi.dataPeriod->setCurrentIndex(d->configGroup.readEntry("dataPeriod", "1").toInt());
   d->configUi.timer->setValue(d->configGroup.readEntry("timer", "5").toInt());

   connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
   parent->setButtons(KDialog::Ok | KDialog::Cancel);
   parent->addPage(widgetConfig, i18n("Configuration"), icon());
}



void Lastmoid::configAccepted()
{
   d->login = d->configUi.user->text();
   d->data = d->configUi.dataType->currentIndex();
   d->period = d->configUi.dataPeriod->currentIndex();
   d->interval = d->configUi.timer->value();

   d->configGroup = config();
   d->configGroup.writeEntry("user", d->login);
   d->configGroup.writeEntry("dataType",QString::number(d->data));
   d->configGroup.writeEntry("dataPeriod",QString::number(d->period));
   d->configGroup.writeEntry("timer",QString::number(d->interval));
   d->configGroup.sync();

   loadConfig();
   clearList();
   d->layout->removeAt(0);
   d->layout->addItem(d->busyWidget);
   d->busyWidget->show();

   d->timer.stop();
   d->avatar = QImage();
   d->state = NotFound; // De-initialise current user
   refresh();
}


void Lastmoid::loadConfig()
{
   static QString dataTable[4] = {
      "recentTracks", "album", "artist", "track" };
   static QString periodTable[5] = {
      "weekly", "overall", "3month", "6month", "12month" };

   d->configGroup = config();
   d->login = d->configGroup.readEntry("user");
   d->data = d->configGroup.readEntry("dataType").toInt();\
   d->dataStr = dataTable[d->data % 4];
   d->period = d->configGroup.readEntry("dataPeriod").toInt();
   d->periodStr = periodTable[d->period % 5];
   d->interval =  QString(d->configGroup.readEntry("timer")).toInt();

   if(d->interval == 0)
      d->interval = 5;
}


void Lastmoid::fetch()
{
   // Is user initialised?
   switch(d->state) {
   case Identified:

      // Recent Tracks
      if(d->data == RecentTracks) {
         d->url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&user="
                     + d->login + "&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
      }
      else {
         // Top dataType
         if(d->period == Weekly)
            d->url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.getweekly"
                        + d->dataStr + "chart&user="
                        + d->login + "&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
         else
            d->url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.gettop"
                        + d->dataStr + "s&user="
                        + d->login + "&period="
                        + d->periodStr +"&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
      }
      break;

   default:
      d->url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.getinfo&user="
                    + d->login + "&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
      break;
   }

   // Execute request
   d->http.setHost(d->url.host());
   d->http.get(d->url.toString());
}

void Lastmoid::httpResponse(int id, bool error)
{
   if(error) {
      qWarning("Received error during HTTP fetch.");
      update();
      return;
   }

   bool result = false;
   switch(d->state) {
   case NotFound:
      result = parseUserData();
      break;

   case Finalizing:
      d->avatar.loadFromData(d->buffer.buffer(), "JPG");
      if(!d->avatar.isNull()) {
         d->avatar = d->avatar.scaledToHeight(60);
         d->state = Identified;
         if(d->buffer.isOpen())
            d->buffer.close();
         refresh();
         result = true;
      }
      break;

   case Identified:
      if(d->data == RecentTracks) result = parseRecentTracks();
      else                        result = parseStatData();
      if(result && d->busyWidget->isVisible()) {
         d->layout->removeAt(0);
         d->layout->addItem(d->scrollWidget);
         d->busyWidget->hide();
      }
      break;

   default:
      break;
   }

   if(result) {
      update();
   }
}

bool Lastmoid::parseStatData()
{

   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;
   doc.setContent(d->http.readAll());
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement(d->dataStr);

   if(d->period == Weekly)
      element = root.firstChildElement("weekly" + d->dataStr + "chart");
   else
      element = root.firstChildElement("top" + d->dataStr + "s");

   // Enter group
   element = element.firstChildElement(d->dataStr);
   if(!element.isNull()) {
      // Clear list
      clearList();
   }
   else {
      return false;
   }

   // Add all elements
   QFontMetrics fnm(font());
   int maxCount = element.firstChildElement("playcount").text().toInt();
   for(;!element.isNull(); element = element.nextSiblingElement(d->dataStr)) {

      BarLabel* label = new BarLabel(d->dataWidget);

      // Fix height mismatch and overflowing
      label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
      label->setMaximumHeight(fnm.height());
      switch(d->data) {
      case TopAlbums:
      case TopTracks:
         label->setText(QString(" %1 - %2")
                        .arg(element.firstChildElement("artist").firstChildElement("name").text())
                        .arg(element.firstChildElement("name").text()));
         break;
      case TopArtists:
         label->setText(element.firstChildElement("name").text());
         break;
      default:
         break;
   }

      label->setBar(element.firstChildElement("playcount").text().toInt() / (float) maxCount);
      d->dataLayout->addItem(label);
   }

   return true;
}


bool Lastmoid::parseRecentTracks()
{

   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;

   doc.setContent (d->http.readAll());
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement("recenttracks");
   element = element.firstChildElement("track");

   // Check expected element
   if(!element.isNull()) {
      // Clear list
      clearList();
   }
   else {
      return false;
   }

   // Add new items
   QFontMetrics fnm(font());
   for (bool flip = true; !element.isNull(); element = element.nextSiblingElement("track")) {

      BarLabel* label = new BarLabel(d->dataWidget);

      // Fix height mismatch and overflowing
      label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
      label->setMaximumHeight(fnm.height());
      label->setText(QString(" %1 - %2")
                     .arg(element.firstChildElement("artist").text())
                     .arg(element.firstChildElement("name").text()));

      // Flip-flop
      if((flip = !flip))
         label->setBar(1.0);

      d->dataLayout->addItem(label);
   }

   return true;
}

bool Lastmoid::parseUserData()
{

   QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
   QDomElement root, element;

   doc.setContent (d->http.readAll());
   root = doc.firstChildElement("lfm");
   element = root.firstChildElement("user");
   element = element.firstChildElement("image");

   if(!element.isNull()) {
      d->state = Finalizing;
      d->buffer.setData(QByteArray());
      d->buffer.open(QBuffer::ReadWrite);
      d->url.setUrl(element.text());
      d->http.setHost(d->url.host());
      d->http.get(d->url.toString(), &d->buffer);
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

   // User string
   p->save();
   QPoint headerPt(contentsRect.topLeft());
   QFont headerFont(font);
   font.setPixelSize(8);
   headerPt.setX(hAlign);
   headerPt.setY(headerPt.y() + d->svgLogo.size().height() + 2);
   p->setFont(headerFont);
   p->setPen(QColor(213,13,6));
   p->drawText(headerPt, d->login);

   // Avatar
   QPainterPath avClip;
   QRect avFrame((int)contentsRect.left() + widgetWidth - 60, (int)contentsRect.top(), 60, 60);
   avClip.addRoundedRect(QRectF(avFrame), 10.0, 10.0);
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
   p->restore();
}

void Lastmoid::clearList()
{
   while(d->dataLayout->count()) {
      QGraphicsLayoutItem* item = d->dataLayout->itemAt(0);
      d->dataLayout->removeAt(0);
      delete item;
   }

   d->dataLayout->invalidate();
}

#include "lastmoid.moc"
