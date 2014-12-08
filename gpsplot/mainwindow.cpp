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
#include <QDebug>
#include <QString>
#include <time.h>

#include <nmea/nmea.h>

#include "gps_info.h"
#include "mainwindow.h"
#include "nmea_client.h"

#include "ui_mainwindow.h"

void trace(const char *str, int str_size)
{
    (void) str_size;

    fprintf(stderr, "Trace: %s\n", str);
}
void error(const char *str, int str_size)
{
    (void) str_size;
    fprintf(stderr, "Error: %s\n", str);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    client = new NmeaClient("localhost", 45000);
    connect(client, SIGNAL(newMessage(QString)), this,
            SLOT(processNmeaMessage(QString)));

    // NMEA parser stuff
    //nmea_property()->trace_func = &trace;
    //nmea_property()->error_func = &error;

    timestamp = 0;
    nmea_zero_INFO(&nmea_info);
    nmea_parser_init(&nmea_parser);

}

MainWindow::~MainWindow()
{
    nmea_parser_destroy(&nmea_parser);

    delete client;
    delete ui;
}

time_t _gps_time(nmeaINFO * info)
{
    struct tm t;

    t.tm_year = info->utc.year;     // since 1900
    t.tm_mon  = info->utc.mon;      // since january
    t.tm_mday = info->utc.day;
    t.tm_hour = info->utc.hour;
    t.tm_min  = info->utc.min;
    t.tm_sec  = info->utc.sec;

    return mktime(&t);
}

/**
 * @brief Process incoming NMEA message.
 * @param nmea_msg The incoming NMEA message.
 */
void MainWindow::processNmeaMessage(QString nmea_msg)
{
    QByteArray ba = nmea_msg.toLocal8Bit();
    nmeaPOS nmea_pos;

    nmea_parse(&nmea_parser, ba.data(), ba.size(), &nmea_info);
    nmea_info2pos(&nmea_info, &nmea_pos);

    time_t current_time = _gps_time(&nmea_info);

    if (current_time != timestamp)
    {
        timestamp = current_time;
        ui->gpsInfoText->setLatLonAlt(nmea_radian2degree(nmea_pos.lat),
                                        nmea_radian2degree(nmea_pos.lon),
                                        nmea_info.elv);
        ui->gpsInfoText->setStatus(nmea_info.sig, nmea_info.fix);

        qDebug() << current_time << ":" << nmea_info.sig << nmea_info.fix <<
                    nmea_radian2degree(nmea_pos.lat) <<
                    nmea_radian2degree(nmea_pos.lon) <<
                    nmea_info.elv << nmea_info.speed << nmea_info.direction;

    }
}
