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
#ifndef GPS_INFO_H
#define GPS_INFO_H

#include <QWidget>

namespace Ui {
class GpsInfo;
}

class GpsInfo : public QWidget
{
    Q_OBJECT

public:
    explicit GpsInfo(QWidget *parent = 0);
    ~GpsInfo();

    void setLatLonAlt(double lat, double lon, double alt);
    void setStatus(int sig, int fix);

private:
    Ui::GpsInfo *ui;
};

#endif // GPS_INFO_H
