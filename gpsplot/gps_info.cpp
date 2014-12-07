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
#include "gps_info.h"
#include "ui_gps_info.h"

GpsInfo::GpsInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GpsInfo)
{
    ui->setupUi(this);
}

GpsInfo::~GpsInfo()
{
    delete ui;
}

void GpsInfo::setLatLonAlt(double lat, double lon, double alt)
{
    ui->gpsLatValue->setText(QString("%1 °").arg(lat));
    ui->gpsLonValue->setText(QString("%1 °").arg(lon));
    ui->gpsAltValue->setText(QString("%1 m").arg(alt));
}


const char * nmea_sig_str[] = {"BAD", "LOW", "MID", "HIGH"};
const char * nmea_fix_str[] = {"?", "BAD", "2D", "3D"};

void GpsInfo::setStatus(int sig, int fix)
{
    if (sig >= 0 && sig <= 3)
        ui->gpsSigValue->setText(nmea_sig_str[sig]);

    if (fix >= 0 && fix <= 3)
        ui->gpsFixValue->setText(nmea_fix_str[fix]);
}
