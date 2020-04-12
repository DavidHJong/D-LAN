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

#include <Libs/qtservice/src/qtservice.h>

#include <Common/Uncopyable.h>
#include <Common/ConsoleReader.h>

#include <CoreApplication.h>
#include <Core.h>

namespace CoreSpace
{
   class CoreService : public QObject, public QtService<CoreApplication>, Common::Uncopyable
   {
      Q_OBJECT
   public:
      CoreService(bool resetSettings, QLocale locale, int argc, char** argv);
      virtual ~CoreService();

      void changePassword(const QString& newPassword);
      void removePassword();

   protected:
      void start() override;
      void stop() override;

      int executeApplication() override;

   private slots:
      void processUserInput(QString);

   private:
      static void printCommands();

      Core* core;

      bool consoleSupport;
      Common::ConsoleReader* consoleReader;
   };
}
