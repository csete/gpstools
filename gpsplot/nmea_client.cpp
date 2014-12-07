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
#include <QByteArray>
#include <QString>

#include "nmea_client.h"

NmeaClient::NmeaClient(QString _host, quint16 _port, QObject *parent) :
    QObject(parent)
{
    host = _host;
    port = _port;
    is_connected = false;
    is_running = false;

    // create socket and establish connection
    socket = new QTcpSocket(parent);
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    socket->connectToHost(host, port, QIODevice::ReadWrite);

}


NmeaClient::~NmeaClient()
{
    is_running = false;

    if (is_connected)
    {
        socket->disconnectFromHost();
        socket->close();
    }

    delete socket;
}


/**
 * @brief Connection notification.
 *
 * This slot is used to receive notification when the TCP connection has been
 * established.
 */
void NmeaClient::connected(void)
{
    qDebug() << __func__;
    is_connected = true;
}

/**
 * @brief Disconnect notification.
 *
 * This slot is activated when the connection to the NMEA server is lost. If
 * this is because of a network error, we try to reconnect.
 */
void NmeaClient::disconnected(void)
{
    qDebug() << __func__;
    is_connected = false;

    // if still running, try to reconnect
    if (is_running)
    {
        socket->connectToHost(host, port, QIODevice::ReadWrite);
    }
}

/**
 * @brief Slot activated when NMEA data is avaialble.
 */
void NmeaClient::dataAvailable(void)
{
    QByteArray data = socket->readAll();

    // FIXME: we should probalby fix the lack of \r in gps2net
    QString data_str = QString(data).trimmed().append("\r\n");

    //qDebug() << __func__ << ":" << data_str;

    emit newMessage(data_str);

}
