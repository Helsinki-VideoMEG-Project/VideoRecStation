/*
 * videowidget.h
 *
 * Author: Andrey Zhdanov
 * Copyright (C) 2014 BioMag Laboratory, Helsinki University Central Hospital
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIDEOWIDGET_H_
#define VIDEOWIDGET_H_

#include <QLabel>
#include <QAtomicInteger>
#include "settings.h"

class VideoWidget : public QLabel
{
    Q_OBJECT

public:
    VideoWidget(QWidget* parent=0);
    QAtomicInteger<bool> rotate{false};

public slots:
    void onDrawFrame(uint8_t* _rawImage, int _width, int _height, bool _color);

private:
    char*       imBuf;
    Settings    settings;
};

#endif /* VIDEOWIDGET_H_ */
