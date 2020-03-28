/**
  * D-LAN - A decentralized LAN file sharing software.
  * Copyright (C) 2010-2012 Greg Burri <greg.burri@gmail.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
  
#pragma once

#include <QApplication>
#include <QTranslator>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QSharedMemory>

#include <MainWindow.h>

#include <Common/RemoteCoreController/Types.h>

namespace GUI
{
   class D_LAN_GUI : public QApplication
   {
      static const QString SHARED_MEMORY_KEYNAME;

      Q_OBJECT
   public:
      class AbortException {};

      D_LAN_GUI(int& argc, char* argv[]);

   protected:
      bool event(QEvent* event);

   private slots:
      void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
      void updateTrayIconMenu();
      void loadLanguage(const QString& filename);
      void mainWindowClosed();
      void showMainWindow();
      void exitGUI();
      void exit(bool stopTheCore = true);

   private:
      QSharedMemory sharedMemory;

      MainWindow* mainWindow;

      QSystemTrayIcon trayIcon;
      QMenu trayIconMenu;

      QSharedPointer<RCC::ICoreConnection> coreConnection;

      QTranslator translator;
   };
}
