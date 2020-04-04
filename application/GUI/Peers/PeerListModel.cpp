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

#include <Peers/PeerListModel.h>
using namespace GUI;

#include <QtAlgorithms>
#include <QStringBuilder>
#include <QSet>

#include <Common/ProtoHelper.h>
#include <Common/Global.h>
#include <Common/StringUtils.h>

#include <Log.h>

/**
  * @class PeerListModel
  *
  * The list of all peers. The list is built from the core state message, see the method 'newState(..)'.
  * The list can be order by the amout of sharing or in an alphabetic way, see the method 'setSortType(..)'.
  */

struct PeerListModel::Peer
{
   bool operator==(const Peer& p) const { return this->peerID == p.peerID; }
   bool operator!=(const Peer& p) const { return this->peerID != p.peerID; }

   Common::Hash peerID;
   QString nick;
   QString coreVersion;
   quint64 sharingAmount;
   QHostAddress ip;
   TransferInformation transferInformation;
   Protos::GUI::State::Peer::PeerStatus status;
};

PeerListModel::PeerListModel(QSharedPointer<RCC::ICoreConnection> coreConnection) :
   coreConnection(coreConnection),
   currentSortType(Protos::GUI::Settings::BY_SHARING_AMOUNT),
   displayOnlyPeersWithStatusOK(false),
   toolTipEnabled(true)
{
   connect(this->coreConnection.data(), SIGNAL(newState(Protos::GUI::State)), this, SLOT(newState(Protos::GUI::State)));
   connect(this->coreConnection.data(), SIGNAL(disconnected(bool)), this, SLOT(coreDisconnected(bool)));
}

PeerListModel::~PeerListModel()
{
   auto end = this->orderedPeers.end();
   for (auto i = this->orderedPeers.begin(); i != end; ++i)
      delete *i;
}

/**
  * Return 'defaultNick' if the peer isn't found.
  */
QString PeerListModel::getNick(const Common::Hash& peerID, const QString& defaultNick) const
{
   Peer* peer = this->indexedPeers.value(peerID, 0);
   if (!peer)
      return defaultNick;
   return peer->nick;
}

QString PeerListModel::getNick(int rowNum) const
{
   if (rowNum >= this->orderedPeers.size())
      return QString();
   return this->orderedPeers.getFromIndex(rowNum)->nick;
}

bool PeerListModel::isOurself(int rowNum) const
{
   if (rowNum >= this->orderedPeers.size())
      return false;
   return this->orderedPeers.getFromIndex(rowNum)->peerID == this->coreConnection->getRemoteID();
}

Common::Hash PeerListModel::getPeerID(int rowNum) const
{
   if (rowNum >= this->orderedPeers.size())
      return Common::Hash();
   return this->orderedPeers.getFromIndex(rowNum)->peerID;
}

QHostAddress PeerListModel::getPeerIP(int rowNum) const
{
   if (rowNum >= this->orderedPeers.size())
      return QHostAddress();
   return this->orderedPeers.getFromIndex(rowNum)->ip;
}

Protos::GUI::State::Peer::PeerStatus PeerListModel::getStatus(int rowNum) const
{
   if (rowNum >= this->orderedPeers.size() || rowNum < 0)
      return Protos::GUI::State::Peer::OK;
   return this->orderedPeers.getFromIndex(rowNum)->status;
}

void PeerListModel::setSortType(Protos::GUI::Settings::PeerSortType sortType)
{
   this->currentSortType = sortType;

   emit layoutAboutToBeChanged();
   switch (this->currentSortType)
   {
   case Protos::GUI::Settings::BY_NICK:
      this->orderedPeers.setSortedFunction([](const Peer* p1, const Peer* p2) {
         if (!p1 || !p2)
            return false;
         const QString& nick1 = Common::StringUtils::toLowerAndRemoveAccents(p1->nick);
         const QString& nick2 = Common::StringUtils::toLowerAndRemoveAccents(p2->nick);
         if (nick1 == nick2)
         {
            if (p1->sharingAmount == p2->sharingAmount)
               return p1->peerID < p2->peerID;
            return p1->sharingAmount > p2->sharingAmount;
         }
         return nick1 < nick2;
      });
      break;

   case Protos::GUI::Settings::BY_SHARING_AMOUNT:
      this->orderedPeers.setSortedFunction([](const Peer* p1, const Peer* p2) {
         if (!p1 || !p2)
            return false;
         if (p1->sharingAmount == p2->sharingAmount)
         {
            const QString& nick1 = Common::StringUtils::toLowerAndRemoveAccents(p1->nick);
            const QString& nick2 = Common::StringUtils::toLowerAndRemoveAccents(p2->nick);
            if (nick1 == nick2)
               return p1->peerID < p2->peerID;
            return nick1 < nick2;
         }
         return p1->sharingAmount > p2->sharingAmount;
      });
      break;

   default:;
   }
   emit layoutChanged();
}

Protos::GUI::Settings::PeerSortType PeerListModel::getSortType() const
{
   return this->currentSortType;
}

void PeerListModel::setDisplayOnlyPeersWithStatusOK(bool displayed)
{
   this->displayOnlyPeersWithStatusOK = displayed;
}

void PeerListModel::setToolTipEnabled(bool enabled)
{
   this->toolTipEnabled = enabled;
}

/**
  * To show only peer from a certain room.
  */
void PeerListModel::setRoom(const QString& room)
{
   this->room = room;
}

void PeerListModel::rmRoom()
{
   this->room = QString();
}

int PeerListModel::rowCount(const QModelIndex& parent) const
{
   return this->orderedPeers.size();
}

int PeerListModel::columnCount(const QModelIndex& parent) const
{
   return 3;
}

QVariant PeerListModel::data(const QModelIndex& index, int role) const
{
   if (!index.isValid() || index.row() >= this->orderedPeers.size())
      return QVariant();

   switch (role)
   {
   case Qt::DisplayRole:
      switch (index.column())
      {
      case 0: return QVariant::fromValue(this->orderedPeers.getFromIndex(index.row())->transferInformation);
      case 1: return this->orderedPeers.getFromIndex(index.row())->nick;
      case 2: return Common::Global::formatByteSize(this->orderedPeers.getFromIndex(index.row())->sharingAmount);
      default: return QVariant();
      }

   case Qt::BackgroundRole:
      if (this->peersToColorize.contains(this->orderedPeers.getFromIndex(index.row())->peerID))
         return this->peersToColorize[this->orderedPeers.getFromIndex(index.row())->peerID];
      if (this->isOurself(index.row()))
         return QColor(192, 255, 192);
      return QVariant();

   case Qt::ForegroundRole:
      if (this->orderedPeers.getFromIndex(index.row())->status != Protos::GUI::State::Peer::OK)
         return QColor(160, 160, 160);
      if (this->peersToColorize.contains(this->orderedPeers.getFromIndex(index.row())->peerID))
         return QColor(255, 255, 255);
      if (this->isOurself(index.row()))
         return QColor(0, 0, 0);
      return QVariant();

   case Qt::TextAlignmentRole:
      return (index.column() == 2 ? Qt::AlignRight : Qt::AlignLeft) + Qt::AlignVCenter;

   case Qt::ToolTipRole:
      if (this->toolTipEnabled)
      {
         const Peer* peer = this->orderedPeers.getFromIndex(index.row());
         const QString coreVersion = peer->coreVersion;
         QString toolTip = peer->nick;
         toolTip.append('\n');

         if (peer->status == Protos::GUI::State::Peer::MORE_RECENT_VERSION)
            toolTip.append(tr("They protocol version is more recent and incompatible with ours. Upgrade you version!")).append('\n');
         else if (peer->status == Protos::GUI::State::Peer::VERSION_OUTDATED)
            toolTip.append(tr("They protocol version is outaded and incompatible with ours. They should upgrade their version!")).append('\n');

         if (!coreVersion.isEmpty())
            toolTip += tr("Version %1\n").arg(coreVersion);
         toolTip +=
            tr("Download rate: ") % Common::Global::formatByteSize(peer->transferInformation.downloadRate) % "/s\n" %
            tr("Upload rate: ") % Common::Global::formatByteSize(peer->transferInformation.uploadRate) % "/s";
         return toolTip;
      }
      else
         return QVariant();

   default:
      return QVariant();
   }
}

void PeerListModel::colorize(const Common::Hash& peerID, const QColor& color)
{
   if (Peer* peer = this->indexedPeers.value(peerID, 0))
   {
      emit layoutAboutToBeChanged();
      this->peersToColorize[peer->peerID] = color;
      emit layoutChanged();
   }
   else
      this->peersToColorize.insert(peerID, color);
}

void PeerListModel::colorize(const QModelIndex& index, const QColor& color)
{
   if (!index.isValid() || index.row() >= this->orderedPeers.size())
      return;

   this->peersToColorize[this->orderedPeers.getFromIndex(index.row())->peerID] = color;

   emit dataChanged(this->createIndex(index.row(), 0), this->createIndex(index.row(), this->columnCount() - 1));
}

void PeerListModel::uncolorize(const QModelIndex& index)
{
   if (!index.isValid() || index.row() >= this->orderedPeers.size())
      return;

   if (this->peersToColorize.remove(this->orderedPeers.getFromIndex(index.row())->peerID))
      emit dataChanged(this->createIndex(index.row(), 0), this->createIndex(index.row(), this->columnCount() - 1));
}

void PeerListModel::newState(const Protos::GUI::State& state)
{
   QSet<Common::Hash> peersDownloadingOurData;
   for (int i = 0; i < state.upload_size(); i++)
      peersDownloadingOurData << Common::Hash(state.upload(i).peer_id().hash());

   QSet<Common::Hash> peersToDisplay;
   if (!this->room.isEmpty())
      for (int i = 0; i < state.room_size(); i++)
         if (state.room(i).name() == this->room.toStdString())
         {
            for (int j = 0; j < state.room(i).peer_id_size(); j++)
               peersToDisplay << Common::Hash(state.room(i).peer_id(j).hash());

            if (state.room(i).joined())
               peersToDisplay << this->coreConnection->getRemoteID();

            break;
         }

   this->updatePeers(state.peer(), peersDownloadingOurData, peersToDisplay);
}

void PeerListModel::coreDisconnected(bool forced)
{
   google::protobuf::RepeatedPtrField<Protos::GUI::State_Peer> peers;
   this->updatePeers(peers);
}

/**
  * @param peersToDisplay If empty then all peers are displayed.
  */
void PeerListModel::updatePeers(const google::protobuf::RepeatedPtrField<Protos::GUI::State::Peer>& peers, const QSet<Common::Hash>& peersDownloadingOurData, const QSet<Common::Hash>& peersToDisplay)
{
   bool dataChanged = false;
   auto setDataChanged =
      [&dataChanged, this]()
      {
         if (!dataChanged)
            emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);
         dataChanged = true;
      };

   auto peersList = this->indexedPeers.keys();
   QSet<Common::Hash> peersToRemove(peersList.begin(), peersList.end());

   for (int i = 0; i < peers.size(); i++)
   {
      const Common::Hash peerID { peers.Get(i).peer_id().hash() };

      // We ignore some peers depending 'peersToDisplay' and 'this->displayOnlyPeersWithStatusOK'.
      if (!peersToDisplay.isEmpty() && !peersToDisplay.contains(peerID) || this->displayOnlyPeersWithStatusOK && peers.Get(i).status() != Protos::GUI::State::Peer::OK)
         continue;

      const QString& nick = Common::ProtoHelper::getStr(peers.Get(i), &Protos::GUI::State::Peer::nick);
      const QString& coreVersion = Common::ProtoHelper::getStr(peers.Get(i), &Protos::GUI::State::Peer::core_version);
      const quint64 sharingAmount = peers.Get(i).sharing_amount();
      const TransferInformation transferInformation { peers.Get(i).download_rate(), peers.Get(i).upload_rate(),  peersDownloadingOurData.contains(peerID) };
      const Protos::GUI::State::Peer::PeerStatus status = peers.Get(i).status();
      const QHostAddress ip =
         peers.Get(i).has_ip() ?
            Common::ProtoHelper::getIP(peers.Get(i).ip()) :
            QHostAddress();

      auto peerIterator = this->indexedPeers.find(peerID);
      Peer* peer = peerIterator == this->indexedPeers.end() ? nullptr : *peerIterator;
      if (peer)
      {
         peersToRemove.remove(peerID);

         if (peer->nick != nick || peer->sharingAmount != sharingAmount || peer->transferInformation != transferInformation)
         {
            setDataChanged();
            this->orderedPeers.remove(peer);
            peer->nick = nick;
            peer->sharingAmount = sharingAmount;
            peer->transferInformation = transferInformation;
            this->orderedPeers.insert(peer);
         }

         peer->ip = ip;
         peer->coreVersion = coreVersion;
         peer->status = status;
      }
      else
      {
         setDataChanged();
         Peer* p = new Peer { peerID, nick, coreVersion, sharingAmount, ip, transferInformation, status };
         this->indexedPeers.insert(peerID, p);
         this->orderedPeers.insert(p);
      }
   }

   QList<Common::Hash> peerIDsRemoved;
   for (auto i = peersToRemove.begin(); i != peersToRemove.end(); ++i)
   {
      setDataChanged();
      Peer* peer = this->indexedPeers[*i];
      peerIDsRemoved << peer->peerID;
      this->indexedPeers.remove(peer->peerID);
      this->orderedPeers.remove(peer);
      delete peer;
   }

   if (dataChanged)
      emit layoutChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

   if (!peerIDsRemoved.isEmpty())
      emit peersRemoved(peerIDsRemoved);
}
