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

#include <QObject>
#include <QTextStream>

#if defined Q_OS_UNIX

#include <QIODevice>
#include <QSocketNotifier>

namespace Common
{
   class ConsoleReader : public QObject
   {
      Q_OBJECT
   public:
      explicit ConsoleReader(QObject* parent = nullptr);

   signals:
      void newLine(QString line);

   private slots:
      void inputAvailable();

   private:
      QTextStream inputStream;
      QSocketNotifier notifier;
   };
}

#elif defined Q_OS_WIN32

#include <QThread>

namespace Common
{
   class ConsoleReader : public QObject
   {
      Q_OBJECT
   public:
      explicit ConsoleReader(QObject* parent = nullptr);

   signals:
      void newLine(QString line);
      void readNextLine();

   private slots:
      void nextLine(QString line);

   private:
      QThread readerThread;
   };

   // Not public and only on Windows.
   class Reader : public QObject
   {
      Q_OBJECT
   public:
      Reader();

   public slots:
      void readLine();

   signals:
       void lineRead(const QString& line);

   private:
       QTextStream inputStream;
   };
}

#endif
