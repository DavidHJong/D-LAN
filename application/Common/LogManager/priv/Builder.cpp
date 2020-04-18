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
  
#include <Builder.h>
using namespace LM;

#include <Constants.h>

#include <ILogger.h>
#include <priv/Logger.h>
#include <priv/Entry.h>
#include <priv/LoggerHook.h>

#include <priv/QtLogger.h>
#include <priv/StdLogger.h>

void Builder::setLogDirName(const QString& logDirName)
{
   Logger::setLogDirName(logDirName);
}

/**
  * Create a new logger, the name may correspond to a module name.
  * @exception LoggerAlreadyExistsException
  */
QSharedPointer<ILogger> Builder::newLogger(const QString& name)
{
   return QSharedPointer<ILogger>(new Logger(name));
}

/**
  * Return an hook to grep all log message for the given severities.
  */
QSharedPointer<ILoggerHook> Builder::newLoggerHook(Severity severities)
{
   QSharedPointer<LoggerHook> loggerHook(new LoggerHook(severities));
   Logger::addALoggerHook(loggerHook);
   return loggerHook;
}

/**
  * Read a log entry given as a string.
  * @exception MalformedEntryLog
  */
QSharedPointer<IEntry> Builder::decode(const QString& line)
{
   return QSharedPointer<IEntry>(new Entry(line));
}

QSharedPointer<IEntry> Builder::newEntry(const QDateTime& dateTime, Severity severity, const QString& message, const QString& name, const QString& thread, const QString& source)
{
   return QSharedPointer<IEntry>(new Entry(dateTime, severity, name, thread, source, message));
}

/**
  * If the Qt message handler is overridden you can redefine it
  * by calling this function.
  * For example the function 'QTest::qExec(..)' will override the message handler.
  */
void Builder::initMsgHandler()
{
   StdLogger::init();
   QtLogger::initMsgHandler();
}
