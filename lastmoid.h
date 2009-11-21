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
 

#ifndef Plasmoid_HEADER
#define Plasmoid_HEADER


#include <KIcon>
#include <Plasma/Applet>
#include <Plasma/Svg>
#include <QHttp>
#include <QString>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <KConfigGroup>

#include "ui_lastmoidConfig.h"


class QSizeF;
 

class Lastmoid : public Plasma::Applet
{
    Q_OBJECT
    public:

        Lastmoid(QObject *parent, const QVariantList &args);
        ~Lastmoid();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,const QRect& contentsRect);
	void fetch();
	void parseXmlTop();
	void parseXmlRecentTracks();
	void loadConfig();



    public slots:

	void finished(int id, bool error);
	void refresh();

    protected slots:
        void configAccepted();

    protected:
        void createConfigurationInterface(KConfigDialog *parent);

    private:

 	Ui::lastmoidConfig uiConfig;
        Plasma::Svg m_svg;
        KIcon m_icon;
	KConfigGroup configGroup;
	
	int connectionId;
	int nbDatas;
	int updateFrequency;
	QString lastUser;
	QString dataType;  
	QString dataPeriod;  
	QUrl url;
	QHttp http;
	QTimer *timer;
	QList<QStringList> datas;
};
K_EXPORT_PLASMA_APPLET(lastmoid, Lastmoid)
#endif