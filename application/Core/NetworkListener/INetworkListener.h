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

#include <Protos/core_protocol.pb.h>

#include <Common/Network/Message.h>

namespace NL
{
   class ISearch;

   class INetworkListener : public QObject
   {
      Q_OBJECT
   public:
      virtual ~INetworkListener() {}

      virtual QSharedPointer<ISearch> newSearch() = 0;

      /**
        * This is needed when sockets have to be rebound.
        * On Windows after disable/enable the netowrk interface, the sockets have to be rebound.
        */
      virtual void rebindSockets() = 0;

      enum class SendStatus
      {
         OK,
         PEER_UNKNOWN,
         MESSAGE_TOO_LARGE,
         UNABLE_TO_SEND
      };

      /**
        * Send a message to a particular peer, if the peer ID isn't given the message is sent to everyone.
        */
      virtual SendStatus send(Common::MessageHeader::MessageType type, const google::protobuf::Message& message, const Common::Hash& peerID = Common::Hash()) = 0;

   signals:
      void received(const Common::Message& message);
      void IMAliveMessageToBeSend(Protos::Core::IMAlive& IMAliveMessage);
   };
}
