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
#include <QSharedPointer>
#include <QList>
#include <QTcpServer>
#include <QLocale>

#include <Common/Uncopyable.h>
#include <Core/FileManager/IFileManager.h>
#include <Core/PeerManager/IPeerManager.h>
#include <Core/UploadManager/IUploadManager.h>
#include <Core/DownloadManager/IDownloadManager.h>
#include <Core/NetworkListener/INetworkListener.h>
#include <Core/ChatSystem/IChatSystem.h>

#include <IRemoteControlManager.h>
#include <priv/RemoteConnection.h>
#include <priv/Log.h>

namespace RCM
{
   class RemoteControlManager : public IRemoteControlManager, Common::Uncopyable
   {
      Q_OBJECT
   public:
      RemoteControlManager(
         QSharedPointer<FM::IFileManager> fileManager,
         QSharedPointer<PM::IPeerManager> peerManager,
         QSharedPointer<UM::IUploadManager> uploadManager,
         QSharedPointer<DM::IDownloadManager> downloadManager,
         QSharedPointer<NL::INetworkListener> networkListener,
         QSharedPointer<CS::IChatSystem> chatSystem
      );

      ~RemoteControlManager();

   private slots:
      void newConnection();
      void connectionDeleted(RemoteConnection* sender);

   private:      
      LOG_INIT_H("RemoteControlManager")

      QSharedPointer<FM::IFileManager> fileManager;
      QSharedPointer<PM::IPeerManager> peerManager;
      QSharedPointer<UM::IUploadManager> uploadManager;
      QSharedPointer<DM::IDownloadManager> downloadManager;
      QSharedPointer<NL::INetworkListener> networkListener;
      QSharedPointer<CS::IChatSystem> chatSystem;

      QTcpServer tcpServerIPv4;
      QTcpServer tcpServerIPv6;
      QList<RemoteConnection*> connections;
   };
}
