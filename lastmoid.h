/***************************************************************************
 *   Copyright (C) 2008 by Damien Lévin <dml_aon@hotmail.com>      	   *
 *                 2009 by Marek Vavrusa <marek@vavrusa.com>      	   *
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
 
#ifndef LASTMOID_H
#define LASTMOID_H

#include <Plasma/Applet>
#include <QByteArray>
#include <QUrl>
#include <KConfigGroup>

class Lastmoid : public Plasma::Applet
{
    Q_OBJECT
    public:

        Lastmoid(QObject *parent, const QVariantList &args);
        ~Lastmoid();

        void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,const QRect& contentsRect);
        void fetch();
        void loadConfig();

    public slots:
        void init();
        void refresh();
        void httpQuery(const QUrl& url);
        void httpResponse(int id, bool error);

    protected slots:
        void configAccepted();
        void setBusy(bool val);

    protected:
        bool parseUserData(const QByteArray& data);
        bool parseStatData(const QByteArray& data);
        bool parseRecentTracks(const QByteArray& data);
        void createConfigurationInterface(KConfigDialog *parent);

    private:
        struct Private;
        Private *d;
};

K_EXPORT_PLASMA_APPLET(lastmoid, Lastmoid)
#endif
