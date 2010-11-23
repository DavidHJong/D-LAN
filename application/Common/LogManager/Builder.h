#ifndef LOGMANAGER_BUILDER_H
#define LOGMANAGER_BUILDER_H

#include <QDir>
#include <QSharedPointer>

#include "IEntry.h"
#include "ILogger.h"
#include "ILoggerHook.h"

namespace LM
{
   class Builder
   {
   public:
      static void setLogDirName(const QString& logDirName);
      static QSharedPointer<ILogger> newLogger(const QString& name);
      static QSharedPointer<ILoggerHook> newLoggerHook(Severity severities);

      static QSharedPointer<IEntry> decode(const QString& line);
      static QSharedPointer<IEntry> newEntry(const QDateTime& dateTime, Severity severity, const QString& message, const QString& name = QString(""), const QString& thread = QString(""), const QString& source = QString(""));

      static void initMsgHandler();
   };
}

// Some useful macros.
#ifdef DEBUG
   #define LOG_USER(logger, mess) logger->log((mess), LM::SV_END_USER, __FILE__, __LINE__)
   #define LOG_DEBU(logger, mess) logger->log((mess), LM::SV_DEBUG, __FILE__, __LINE__)
   #define LOG_WARN(logger, mess) logger->log((mess), LM::SV_WARNING, __FILE__, __LINE__)
   #define LOG_ERRO(logger, mess) logger->log((mess), LM::SV_ERROR, __FILE__, __LINE__)
   #define LOG_FATA(logger, mess) logger->log((mess), LM::SV_FATAL_ERROR, __FILE__, __LINE__)
#else
   #define LOG_USER(logger, mess) logger->log((mess), LM::SV_END_USER)
   #define LOG_DEBU(logger, mess)
   #define LOG_WARN(logger, mess) logger->log((mess), LM::SV_WARNING)
   #define LOG_ERRO(logger, mess) logger->log((mess), LM::SV_ERROR)
   #define LOG_FATA(logger, mess) logger->log((mess), LM::SV_FATAL_ERROR)
#endif

#endif
