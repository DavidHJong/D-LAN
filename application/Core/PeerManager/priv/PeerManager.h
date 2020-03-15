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
#include <QMap>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QList>
#include <QTcpSocket>

#include <Common/Hash.h>
#include <Common/Uncopyable.h>

#include <Core/FileManager/IFileManager.h>

#include <IPeerManager.h>
#include <Common/LogManager/ILogger.h>
#include <priv/Peer.h>
#include <priv/PeerSelf.h>
#include <priv/Log.h>

namespace PM
{
   class Peer;

   struct PendingSocket
   {
      PendingSocket(QTcpSocket* socket) : socket(socket) { this->t.start(); }

      QTcpSocket* socket;
      QTime t;
   };

   class PeerManager : public IPeerManager, Common::Uncopyable
   {
      Q_OBJECT
   public:
      PeerManager(QSharedPointer<FM::IFileManager> fileManager);
      ~PeerManager();

      void setNick(const QString& nick);

      IPeer* getSelf();
      int getNbOfPeers() const;
      QList<IPeer*> getPeers() const;

      IPeer* getPeer(const Common::Hash& ID);
      IPeer* createPeer(const Common::Hash& ID, const QString& nick);

      void updatePeer(
         const Common::Hash& ID,
         const QHostAddress& IP,
         quint16 port,
         const QString& nick,
         const quint64& sharingAmount,
         const QString& coreVersion,
         quint32 downloadRate,
         quint32 uploadRate,
         quint32 protocolVersion
      );

      void removePeer(const Common::Hash& ID, const QHostAddress& IP);
      void removeAllPeers();
      void newConnection(QTcpSocket* tcpSocket);

      void onGetChunk(QSharedPointer<FM::IChunk> chunk, int offset, QSharedPointer<PeerMessageSocket> socket);

   private slots:
      void dataReceived(QTcpSocket* tcpSocket = nullptr);
      void disconnected(QTcpSocket* tcpSocket = nullptr);
      void checkIdlePendingSockets();
      void peerUnblocked();

   private:
      void removeFromPending(QTcpSocket* socket);

      LOG_INIT_H("PeerManager")

      QSharedPointer<FM::IFileManager> fileManager;

      PeerSelf* self; // Ourself.
      QMap<Common::Hash, Peer*> peers; // The other peers.

      QTimer timer; ///< Used to check periodically if some pending sockets have timeouted.
      QList<PendingSocket> pendingSockets;
   };
}
