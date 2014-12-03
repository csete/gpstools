/* -*- c++ -*- */
/*
 * gpsplot: Simple application to plot GPS data.
 *
 * Copyright 2014 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef NMEA_CLIENT_H
#define NMEA_CLIENT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>

/**
 * @brief TCP client used to receiver NMEA senteces from a remote server.
 */
class NmeaClient : public QObject
{
    Q_OBJECT

public:
    explicit NmeaClient(QString _host, quint16 _port, QObject *parent = 0);
    ~NmeaClient();

signals:
    void newMessage(QString message);

private slots:
    void connected(void);
    void disconnected(void); // FIXME: not used.
    void dataAvailable(void);


private:
    QString     host;
    quint16     port;
    QTcpSocket *socket;

    bool is_connected;
    bool is_running;
};

#endif // NMEA_CLIENT_H
