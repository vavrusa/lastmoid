/***************************************************************************
 *   Copyright (C) 2008 by Damien Lévin <dml_aon@hotmail.com>      	   *
 *                                                                         *
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


#include "lastmoid.h"


#include <plasma/svg.h>
#include <plasma/theme.h>
#include <QDebug> 
#include <QDomDocument>
#include <QDomElement>
#include <QRectF>
#include <QPainter>
#include <KConfigDialog>


Lastmoid::Lastmoid(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args) 

{

    m_svg.setImagePath("widgets/lastmoid");
    timer = new QTimer(this);

    connect(&http, SIGNAL(requestFinished(int, bool)),this, SLOT(finished(int, bool)));
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    
    setBackgroundHints(DefaultBackground);
    resize(200, 230);

    refresh();

}
 
 
Lastmoid::~Lastmoid()
{
   timer->stop();
   delete timer;
}


void Lastmoid::refresh()
{
   fetch();
   timer->stop();
   timer->setInterval(updateFrequency * 60 * 1000);
   timer->start();
}


void Lastmoid::createConfigurationInterface(KConfigDialog *parent)
{

	QWidget *widgetConfig = new QWidget;
	configGroup = config();
	uiConfig.setupUi(widgetConfig);
	uiConfig.user->setText(configGroup.readEntry("user"));
	uiConfig.nbDatas->setValue(QString(configGroup.readEntry("nbDatas")).toInt());
	uiConfig.dataType->setCurrentIndex(uiConfig.dataType->findText(configGroup.readEntry("dataType")));
	uiConfig.dataPeriod->setCurrentIndex(uiConfig.dataPeriod->findText(configGroup.readEntry("dataPeriod")));

	connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
	parent->setButtons(KDialog::Ok | KDialog::Cancel);
	parent->addPage(widgetConfig, i18n("Configuration"), icon());

}



void Lastmoid::configAccepted(){
    
    lastUser = uiConfig.user->text();
    nbDatas = uiConfig.nbDatas->value();
    dataType = uiConfig.dataType->currentText();
    dataPeriod = uiConfig.dataPeriod->currentText();
    updateFrequency = uiConfig.timer->value();

    configGroup = config();
    configGroup.writeEntry("user", lastUser);
    configGroup.writeEntry("nbDatas",QString::number(nbDatas));
    configGroup.writeEntry("dataType",dataType);
    configGroup.writeEntry("dataPeriod",dataPeriod);
    configGroup.writeEntry("timer",QString::number(updateFrequency));

    
    refresh();
}


void Lastmoid::loadConfig(){

    configGroup = config();
    lastUser = configGroup.readEntry("user");
    nbDatas = QString(configGroup.readEntry("nbDatas")).toInt();
    dataType = configGroup.readEntry("dataType");
    dataPeriod = configGroup.readEntry("dataPeriod");
    updateFrequency =  QString(configGroup.readEntry("timer")).toInt();

    if (updateFrequency==0){
      updateFrequency=5; 
    }

}


void Lastmoid::fetch()
 {

    loadConfig();

    if (QString::compare(dataType,QString("recentTracks"))==0){
      //Recent Tracks
      url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&user="+lastUser+"&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
    }else{
      //Top dataType
      if (QString::compare(dataPeriod,QString("weekly"))==0){
	  url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.getweekly"+dataType+"chart&user="+lastUser+"&api_key=b6eb61f91b89e55548dd14732ee0b8a1");
      }else{
	  url.setUrl("http://ws.audioscrobbler.com/2.0/?method=user.gettop"+dataType+"s&user="+lastUser+"&period="+dataPeriod+"&api_key=b6eb61f91b89e55548dd14732ee0b8a1");   
      }
    }

    datas.clear();

    http.setHost(url.host());
    connectionId = http.get(url.toString ());
 }


 void Lastmoid::finished(int id, bool error)
 {
     if (error) {
         qWarning("Received error during HTTP fetch.");
     }else{
      if (QString::compare(dataType,QString("recentTracks"))==0){
	  //Recent Tracks
	  parseXmlRecentTracks();
      }else{
	  //Top dataType
	  parseXmlTop();
      }
    }
    update();
 }

void Lastmoid::parseXmlTop()
 {
 
    QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
    QDomElement name,playcount,root,element;

    doc.setContent (http.readAll());
    
    root = doc.firstChildElement("lfm");  
    element = root.firstChildElement(dataType);

    if (dataPeriod =="weekly"){
       element = root.firstChildElement("weekly"+dataType+"chart");
    }else{
       element = root.firstChildElement("top"+dataType+"s");
    }
    element = element.firstChildElement(dataType);
  
  
    for (; !element.isNull(); element = element.nextSiblingElement(dataType)) {

	QStringList item;
	
	name = element.firstChildElement("name");
	playcount = element.firstChildElement("playcount");

	item.append(element.attribute("rank"));
	item.append(name.text());
	item.append(playcount.text());

	datas.append(item);

   }

 }


void Lastmoid::parseXmlRecentTracks()
{
  
    QDomDocument doc("?xml version=\"1.0\" encoding=\"utf-8\" ?");
    QDomElement artist,title,album,root,element;

    doc.setContent (http.readAll());
    
    root = doc.firstChildElement("lfm");  
    element = root.firstChildElement("recenttracks");
    element = element.firstChildElement("track");
  
    for (; !element.isNull(); element = element.nextSiblingElement("track")) {

	QStringList item;

	artist = element.firstChildElement("artist");
	title = element.firstChildElement("name");
	album = element.firstChildElement("album");

	item.append(artist.text());
	item.append(title.text());
	item.append(album.text());
    
	datas.append(item);

   }
 }


void Lastmoid::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{


    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);


    int i=1,widgetWidth,widgetHeight,vAlign,hAlign,maxListening=0;
    QRectF rect;

    widgetWidth = (int)contentsRect.width();
    widgetHeight = (int)contentsRect.height()-60;
    
    
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);
 

     hAlign = widgetWidth/10;
     m_svg.resize(94, 48);
     m_svg.paint(p,hAlign, (int)contentsRect.top());    

    //Get maxListening 
    if(datas.size()>=1){	
	maxListening=datas.at(0).at(2).toInt();
    }


if (QString::compare(dataType,QString("recentTracks"))==0){
//Recent Tracks display
    if (nbDatas>10){
	nbDatas=10; 
    }
    for (i=0;i < nbDatas;i++){     

      if((datas.size() > i) ){

	vAlign = (i+1)*widgetHeight/nbDatas+60;
	
	rect = QRectF(hAlign,vAlign,widgetWidth-hAlign,-widgetHeight/nbDatas*0.75);
	if (i%2==1){
	  p->fillRect(rect,QBrush(QColor(234, 234, 234, 127)));
	}else{
	  p->fillRect(rect,QBrush(QColor(215, 0, 25, 200)));
	}
	p->setPen(Qt::black);
	font.setPixelSize((int)(widgetHeight/nbDatas*0.5)); 
	p->setFont(font);
	p->drawText(rect,Qt::AlignLeft | Qt::AlignVCenter| Qt::TextWordWrap ,"  "+datas.at(i).at(0)+" - "+datas.at(i).at(1));
	p->restore();
	
      }
    }
}else{
//Top display
    for (i=0;i < nbDatas;i++){     

      if((datas.size() > i) ){

	vAlign = (i+1)*widgetHeight/nbDatas+60;
	
	rect = QRectF(hAlign,vAlign,widgetWidth-hAlign,-widgetHeight/nbDatas*0.75);
	p->fillRect(rect,QBrush(QColor(234, 234, 234, 127)));

	rect = QRectF(hAlign,vAlign,(widgetWidth-hAlign)*datas.at(i).at(2).toInt()/maxListening,-widgetHeight/nbDatas*0.75);
	p->fillRect(rect,QBrush(QColor(215, 0, 25, 200)));

	rect = QRectF(hAlign,vAlign,widgetWidth-hAlign,-widgetHeight/nbDatas*0.75);
	p->setPen(Qt::black);
	font.setPixelSize((int)(widgetHeight/nbDatas*0.5)); 
	p->setFont(font);
	p->drawText(rect,Qt::AlignLeft | Qt::AlignVCenter| Qt::TextWordWrap ,"  "+datas.at(i).at(0)+" - "+datas.at(i).at(1)+" ("+datas.at(i).at(2)+")");

	p->restore();
	
      }
    }
}

}
 
#include "lastmoid.moc"