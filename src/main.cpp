/*
 * main.cpp
 *
 * Author: Andrey Zhdanov
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

#include <iostream>
#include <QtGui>
#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>

#include "maindialog.h"
#include "settings.h"

using namespace std;

bool checkSingleInstance(const QString& serverName)
{
    QLocalSocket socket;
    socket.connectToServer(serverName);
    
    if (socket.waitForConnected(1000)) {
        // Another instance is running
        QMessageBox::warning(nullptr, "VideoRecStation Already Running",
                            "Another instance VideoRecStation is already running.");
        return false;
    }
    
    // No instance running, create server
    QLocalServer::removeServer(serverName); // Clean up if previous crashed
    QLocalServer* server = new QLocalServer();
    return server->listen(serverName);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString serverName = LOCAL_SERVER_NAME;
    
    if (!checkSingleInstance(serverName)) {
        return 0; // Exit if another instance exists
    }

    MainDialog w;

    w.show();
    return a.exec();
}
