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
  
#include <priv/Peer.h>
using namespace PM;

#include <limits>

#include <Common/Settings.h>
#include <Common/Global.h>
#include <Common/LogManager/Builder.h>

#include <priv/PeerManager.h>
#include <priv/Constants.h>
#include <priv/Log.h>
#include <priv/GetEntriesResult.h>
#include <priv/GetHashesResult.h>
#include <priv/GetChunksResult.h>

const quint32 Peer::MAX_SPEED = std::numeric_limits<quint32>::max();

Peer::Peer(PeerManager* peerManager, QSharedPointer<FM::IFileManager> fileManager, Common::Hash ID, const QString& nick) :
   connectionPool(peerManager, fileManager, ID),
   ID(ID),
   port(0),
   nick(nick),
   sharingAmount(0),
   speed(MAX_SPEED),
   alive(false),
   blocked(false),
   protocolVersion(0)
{
   this->speedTimer.invalidate();

   this->aliveTimer.setSingleShot(true);
   this->aliveTimer.setInterval(SETTINGS.get<double>("peer_timeout_factor") * SETTINGS.get<quint32>("peer_imalive_period"));
   connect(&this->aliveTimer, &QTimer::timeout, this, &Peer::consideredDead);

   this->blockedTimer.setSingleShot(true);
   connect(&this->blockedTimer, &QTimer::timeout, this, &Peer::unblock);
}

/**
  * Mainly for debugging purpose.
  */
QString Peer::toStringLog() const
{
   return QString("%1 %2 %3:%4 %5 %6/s").arg(this->nick).arg(this->ID.toStr()).arg(this->IP.toString()).arg(this->port).arg(this->alive ? "<alive>" : "<dead>").arg(Common::Global::formatByteSize(const_cast<Peer*>(this)->getSpeed(), 4));
}

Common::Hash Peer::getID() const
{
   return this->ID;
}

QHostAddress Peer::getIP() const
{
   return this->IP;
}

quint16 Peer::getPort() const
{
   return this->port;
}

QString Peer::getNick() const
{
   return this->nick;
}

QString Peer::getCoreVersion() const
{
   return this->coreVersion;
}

quint64 Peer::getSharingAmount() const
{
   return this->sharingAmount;
}

quint32 Peer::getDownloadRate() const
{
   return this->downloadRate;
}

quint32 Peer::getUploadRate() const
{
   return this->uploadRate;
}

quint32 Peer::getSpeed()
{
   QMutexLocker locker(&this->mutex);

   // In [ms].
   static const quint32 SPEED_VALIDITY_PERIOD = 1000 * SETTINGS.get<quint32>("download_rate_valid_time_factor") / (SETTINGS.get<quint32>("lan_speed") / 1024 / 1024);

   if (this->speedTimer.isValid() && this->speedTimer.elapsed() > SPEED_VALIDITY_PERIOD)
      this->speed = MAX_SPEED;
   return this->speed;
}

void Peer::setSpeed(quint32 newSpeed)
{
   QMutexLocker locker(&this->mutex);

   this->speedTimer.start();
   if (this->speed == MAX_SPEED)
      this->speed = newSpeed;
   else
      this->speed = (this->speed + newSpeed) / 2;
}

void Peer::block(int duration, const QString& reason)
{
   QMutexLocker locker(&this->mutex);

   this->blocked = true;
   this->blockedReason = reason;
   this->blockedTimer.setInterval(duration);

   QMetaObject::invokeMethod(&this->blockedTimer, "start");
}

bool Peer::isAlive() const
{
   QMutexLocker locker(&this->mutex);
   return this->alive;
}

bool Peer::isAvailable() const
{
   QMutexLocker locker(&this->mutex);
   return this->alive && !this->blocked && this->isVersionCompatible();
}

quint32 Peer::getProtocolVersion() const
{
   QMutexLocker locker(&this->mutex);
   return this->protocolVersion;
}

void Peer::update(
   const QHostAddress& IP,
   quint16 port,
   const QString& nick,
   const quint64& sharingAmount,
   const QString& coreVersion,
   quint32 downloadRate,
   quint32 uploadRate,
   quint32 protocolVersion
)
{
   this->alive = true;
   this->aliveTimer.start();

   this->IP = IP;
   this->port = port;
   this->nick = nick;
   this->coreVersion = coreVersion;
   this->sharingAmount = sharingAmount;
   this->downloadRate = downloadRate;
   this->uploadRate = uploadRate;
   this->protocolVersion = protocolVersion;

   this->connectionPool.setIP(this->IP, this->port);
}

void Peer::setAsDead()
{
   this->aliveTimer.stop();
   this->consideredDead();
}

QSharedPointer<IGetEntriesResult> Peer::getEntries(const Protos::Core::GetEntries& dirs)
{
   if (!this->isAvailable())
      return QSharedPointer<IGetEntriesResult>();

   return QSharedPointer<IGetEntriesResult>(
      new GetEntriesResult(dirs, this->connectionPool.getASocket()),
      &IGetEntriesResult::doDeleteLater
   );
}

QSharedPointer<IGetHashesResult> Peer::getHashes(const Protos::Common::Entry& file)
{
   if (!this->isAvailable())
      return QSharedPointer<IGetHashesResult>();

   return QSharedPointer<IGetHashesResult>(
      new GetHashesResult(file, this->connectionPool.getASocket()),
      &IGetHashesResult::doDeleteLater
   );
}

QSharedPointer<IGetChunkResult> Peer::getChunk(const Protos::Core::GetChunk& chunk)
{
   if (!this->isAvailable())
      return QSharedPointer<IGetChunkResult>();

   return QSharedPointer<IGetChunkResult>(
      new GetChunksResult(chunk, this->connectionPool.getASocket()),
      &IGetChunkResult::doDeleteLater
   );
}

void Peer::newConnexion(QTcpSocket* tcpSocket)
{
   L_DEBU(QString("New Connection from %1").arg(this->toStringLog()));
   this->connectionPool.newConnexion(tcpSocket);
}

void Peer::consideredDead()
{
   L_DEBU(QString("Peer is dead: %1").arg(this->toStringLog()));
   this->connectionPool.closeAllSocket();
   this->alive = false;
}

void Peer::unblock()
{
   this->blocked = false;
   emit unblocked();
}
