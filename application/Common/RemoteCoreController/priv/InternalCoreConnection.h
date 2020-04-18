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
#include <QTcpSocket>
#include <QHostInfo>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QProcess>
#include <QElapsedTimer>
#include <QLocale>

#include <Protos/gui_protocol.pb.h>
#include <Protos/common.pb.h>

#include <Common/Network/MessageSocket.h>
#include <Common/Timeoutable.h>
#include <Common/LogManager/IEntry.h>

#include <ICoreConnection.h>
#include <ISendChatMessageResult.h>
#include <IBrowseResult.h>
#include <ISearchResult.h>
#include <Types.h>
#include <priv/CoreController.h>

namespace RCC
{
   class SendChatMessageResult;
   class BrowseResult;
   class SearchResult;

   class InternalCoreConnection : public Common::MessageSocket
   {
      Q_OBJECT
      static const int NB_RETRIES_MAX;
      static const int TIME_BETWEEN_RETRIES; // [ms]

   protected:
      class Logger : public ILogger
      {
      public:
         void logDebug(const QString& message);
         void logError(const QString& message);
      };

   public:
      InternalCoreConnection(CoreController& coreController);
      ~InternalCoreConnection() override;

      void connectToCore(const QString& address, quint16 port, Common::Hash password);
      void connectToCore(const QString& address, quint16 port, const QString& password);

      bool isLocal() const override;
      bool isConnected() const override;

      void disconnectFromCore();

      QSharedPointer<ISendChatMessageResult> sendChatMessage(int socketTimeout, const QString& message, const QString& roomName = QString(), const QList<Common::Hash>& peerIDsAnswered = QList<Common::Hash>());
      void joinRoom(const QString& room);
      void leaveRoom(const QString& room);
      void setCoreSettings(const Protos::GUI::CoreSettings settings);
      void setCoreLanguage(const QLocale& locale);
      bool setCorePassword(const QString& newPassword, const QString& oldPassword = QString());
      void resetCorePassword();

      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID, int socketTimeout);
      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID, const Protos::Common::Entry& entry, int socketTimeout);
      QSharedPointer<IBrowseResult> browse(const Common::Hash& peerID, const Protos::Common::Entries& entries, bool withRoots, int socketTimeout);

      QSharedPointer<ISearchResult> search(const Protos::Common::FindPattern& findPattern, bool local, int socketTimeout);

      void download(const Common::Hash& peerID, const Protos::Common::Entry& entry);
      void download(const Common::Hash& peerID, const Protos::Common::Entry& entry, const Common::Hash& sharedFolderID, const QString& path = "/");
      void cancelDownloads(const QList<quint64>& downloadIDs, bool complete = false);
      void pauseDownloads(const QList<quint64>& downloadIDs, bool pause = true);
      void moveDownloads(const QList<quint64>& downloadIDRefs, const QList<quint64>& downloadIDs, Protos::GUI::MoveDownloads::Position position);

      void refresh();
      void refreshNetworkInterfaces();

      bool isRunningAsSubProcess() const;
      ICoreConnection::ConnectionInfo getConnectionInfo() const;

   signals:
      void connectingError(RCC::ICoreConnection::ConnectionErrorCode code);
      void connected();
      void disconnected(bool asked); // 'asked' = true if disconnected by 'disconnectFromCore()'.

      void newState(const Protos::GUI::State&);
      void newChatMessages(const Protos::Common::ChatMessages&);
      void newLogMessages(const QList<QSharedPointer<LM::IEntry>>&);

      void browseResult(const Protos::GUI::BrowseResult& browseResult);
      void searchResult(const Protos::Common::FindResult& findResult);

   private slots:
      void addressResolved(QHostInfo hostInfo);
      void tryToConnectToTheNextAddress();
      void stateChanged(QAbstractSocket::SocketState socketState);

   private:      
      void connectedAndAuthenticated();

      void sendCurrentLanguage();

      void onNewMessage(const Common::Message& message) override;
      void onDisconnected() override;

      friend class BrowseResult;
      friend class SearchResult;

      CoreController& coreController;

      ICoreConnection::ConnectionInfo connectionInfo;

      QLocale currentLanguage;

      int currentHostLookupID;

      QList<QHostAddress> addressesToTry; // When a name is resolved many addresses can be returned, we will try all of them until a connection is successfully established.
      QList<QHostAddress> addressesToRetry;
      int nbRetries;

      QList<QWeakPointer<SendChatMessageResult>> sendChatMessageResultWithoutReply;
      QList<QWeakPointer<BrowseResult>> browseResultsWithoutTag;
      QList<QWeakPointer<SearchResult>> searchResultsWithoutTag;

      bool authenticated;
      bool forcedToClose;

      // Temporary text password. Once we got the salt sent by the Core we set 'connectionInfo.password' with the salted password and we erase this member.
      QString password;
      quint64 salt;
   };
}
