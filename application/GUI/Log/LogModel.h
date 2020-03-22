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

#include <QAbstractTableModel>

#include <Common/LogManager/IEntry.h>
#include <Common/LogManager/ILoggerHook.h>
#include <Common/RemoteCoreController/ICoreConnection.h>

namespace GUI
{
   class LogModel : public QAbstractTableModel
   {
      Q_OBJECT
   public:
      LogModel(QSharedPointer<RCC::ICoreConnection> coreConnection);

      int rowCount(const QModelIndex& parent = QModelIndex()) const;
      int columnCount(const QModelIndex& parent = QModelIndex()) const;
      QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

      LM::Severity getSeverity(int row) const;

   private slots:
      void newLogEntry(QSharedPointer<LM::IEntry> entry);
      void newLogEntries(const QList<QSharedPointer<LM::IEntry>>& entries);

   private:
      QSharedPointer<RCC::ICoreConnection> coreConnection;
      QSharedPointer<LM::ILoggerHook> loggerHook;
      QList<QSharedPointer<LM::IEntry>> entries;
   };
}
