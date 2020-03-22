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

#include <Tests.h>
using namespace PM;

#include <QtDebug>
#include <QStringList>

#include <Protos/core_protocol.pb.h>
#include <Protos/core_settings.pb.h>
#include <Protos/common.pb.h>

#include <Common/LogManager/Builder.h>
#include <Common/PersistentData.h>
#include <Common/Constants.h>
#include <Common/Global.h>
#include <Common/Settings.h>

#include <ResultListener.h>
#include <IGetEntriesResult.h>
#include <IGetHashesResult.h>

const int Tests::PORT = 59487;

/**
  * @class Tests
  *
  * Create some fileManager and associated peerManager.
  */

Tests::Tests()
{
}

void Tests::initTestCase()
{
   LM::Builder::initMsgHandler();
   qDebug() << "===== initTestCase() =====";
   try
   {
      QString tempFolder = Common::Global::setCurrentDirToTemp("PeerManagerTests");
      qDebug() << "Application directory path (where the persistent data is put) : " <<  Common::Global::getDataFolder(Common::Global::DataFolderType::LOCAL, false);
      qDebug() << "The file created during this test are put in : " << tempFolder;
   }
   catch (Common::Global::UnableToSetTempDirException& e)
   {
      QFAIL(e.errorMessage.toLatin1().constData());
   }

   Common::PersistentData::rmValue(Common::Constants::FILE_CACHE, Common::Global::DataFolderType::LOCAL); // Reset the stored cache.

   SETTINGS.setFilename("core_settings_peer_manager_tests.txt");
   SETTINGS.setSettingsMessage(new Protos::Core::Settings());

   QVERIFY(this->createInitialFiles());

   this->fileManagers << FM::Builder::newFileManager() << FM::Builder::newFileManager();

   this->peerIDs << Common::Hash::fromStr("1111111111111111111111111111111111111111") <<
                    Common::Hash::fromStr("2222222222222222222222222222222222222222");

   this->peerSharedDirs << "/sharedDirs/peer1" << "/sharedDirs/peer2";

   // 1) Create each peer manager.
   for (int i = 0; i < this->peerIDs.size(); i++)
   {
      SETTINGS.set("peer_id", this->peerIDs[i]);
      QSharedPointer<IPeerManager> peerManager = Builder::newPeerManager(this->fileManagers[i]);
      peerManager->setNick(QString("peer#%1").arg(i + 1));
      this->peerManagers << peerManager;
   }

   // 2) Set the shared directories.
   for (int i = 0; i < this->peerIDs.size(); i++)
   {
      this->fileManagers[i]->setSharedDirs(QStringList() << QDir::currentPath().append(this->peerSharedDirs[i]));
   }

   // 3) Create the peer update (to simulate the periodic update).
   this->peerUpdater = new PeerUpdater(this->fileManagers, this->peerManagers, PORT);

   // 4) Create the servers to listen new TCP connections and forward them to the right peer manager.
   for (int i = 0; i < this->peerIDs.size(); i++)
   {
      this->servers << new TestServer(this->peerManagers[i], PORT + i);
   }
}

void Tests::updatePeers()
{
   qDebug() << "===== updatePeers() =====";

   this->peerUpdater->start();

   // This test shouldn't take less than ~3 s.
   QElapsedTimer timer;
   timer.start();

   // Check if each peer knows the other.
   for (int i = 0; i < this->peerIDs.size(); i++)
   {
      QList<IPeer*> peers = this->peerManagers[i]->getPeers();

      // Wait peer knows other peers.
      if (peers.size() != this->peerIDs.size() - 1)
      {
         i--;
         QTest::qWait(100);
         if (timer.elapsed() > 3000)
            QFAIL("Update peers failed..");
         continue;
      }

      for (int j = 0; j < this->peerIDs.size(); j++)
      {
         if (j == i)
            continue;

         bool found = false;
         for (int k = 0; k < peers.size(); k++)
            if (peers[k]->getID() == this->peerManagers[j]->getSelf()->getID())
            {
               found = true;
               QCOMPARE(peers[k]->getNick(), this->peerManagers[j]->getSelf()->getNick());

               // Wait peer j knows peer k amount (amount increase concurrently during the scanning process).
               if (peers[k]->getSharingAmount() != this->fileManagers[j]->getAmount())
               {
                  k--;
                  QTest::qWait(100);
                  if (timer.elapsed() > 3000)
                     QFAIL("Sharing amount not equals..");
                  continue;
               }
               break;
            }

         QVERIFY(found);
      }
   }
}

void Tests::getPeerFromID()
{
   qDebug() << "===== getPeerFromID() =====";

   for (int i = 0; i < this->peerIDs.size(); i++)
   {
      for (int j = 0; j < this->peerIDs.size(); j++)
      {
         if (j == i)
            continue;
         QCOMPARE(this->peerManagers[i]->getSelf()->getID(), this->peerManagers[j]->getPeer(this->peerManagers[i]->getSelf()->getID())->getID());
      }

      QVERIFY(this->peerManagers[i]->getPeer(Common::Hash::rand()) == 0);
   }
}

/**
  * Peer#1 asking for the root entries of peer#2.
  */
void Tests::askForRootEntries()
{
   qDebug() << "===== askForRootEntries() =====";

   Protos::Core::GetEntries getEntriesMessage;
   QSharedPointer<IGetEntriesResult> result = this->peerManagers[0]->getPeers()[0]->getEntries(getEntriesMessage);

   QVERIFY(!result.isNull());

   connect(result.data(), &IGetEntriesResult::result, &this->resultListener, &ResultListener::entriesResult);
   result->start();

   QElapsedTimer timer;
   timer.start();

   int nbEntriesReceived;
   while ((nbEntriesReceived = this->resultListener.getNbEntriesResultReceived(0)) != 1)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 5000)
         QFAIL(QString("We don't receive the right number of root entry. Number received: %1").arg(nbEntriesReceived).toLatin1());
   }
}

/**
  * Peer#1 browsing the content of the first shared directory of peer#2.
  * Uses the same socket as the previous request.
  */
void Tests::askForSomeEntries()
{
   qDebug() << "===== askForSomeEntries() =====";

   QElapsedTimer timer;

   QVERIFY(!this->resultListener.getEntriesResultList().isEmpty());

   Protos::Core::GetEntries getEntriesMessage1;
   getEntriesMessage1.mutable_dirs()->add_entry()->CopyFrom(this->resultListener.getEntriesResultList().last().result(0).entries().entry(0));
   QSharedPointer<IGetEntriesResult> result1 = this->peerManagers[0]->getPeers()[0]->getEntries(getEntriesMessage1);
   QVERIFY(!result1.isNull());
   connect(result1.data(), &IGetEntriesResult::result, &this->resultListener, &ResultListener::entriesResult);
   result1->start();

   timer.start();
   while (this->resultListener.getNbEntriesResultReceived(0) != 4)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 3000)
         QFAIL("We don't receive the right number of entry after sending 'getEntriesMessage1'.");
   }

   Protos::Core::GetEntries getEntriesMessage2;
   Protos::Common::Entry* entry = getEntriesMessage2.mutable_dirs()->add_entry();
   entry->CopyFrom(this->resultListener.getEntriesResultList().last().result(0).entries().entry(0));
   entry->mutable_shared_dir()->CopyFrom(getEntriesMessage1.dirs().entry(0).shared_dir());
   QSharedPointer<IGetEntriesResult> result2 = this->peerManagers[0]->getPeers()[0]->getEntries(getEntriesMessage2);
   QVERIFY(!result2.isNull());
   connect(result2.data(), &IGetEntriesResult::result, &this->resultListener, &ResultListener::entriesResult);
   result2->start();

   timer.start();
   while (this->resultListener.getNbEntriesResultReceived(0) != 3)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 3000)
         QFAIL("We don't receive the right number of entry after sending 'getEntriesMessage2'.");
   }
}

void Tests::askForHashes()
{
   qDebug() << "===== askForHashes() =====";

   const quint32 NUMBER_OF_CHUNK = 4;
   const quint32 CHUNK_SIZE = Common::Constants::CHUNK_SIZE;

   // 1) Create a big file.
   {
      QFile file("sharedDirs/peer2/big.bin");
      file.open(QIODevice::WriteOnly);

      // To have four different hashes.
      for (quint32 i = 0; i < NUMBER_OF_CHUNK; i++)
      {
         QByteArray randomData(CHUNK_SIZE, i);
         file.write(randomData);
      }
   }

   QElapsedTimer timer;

   // Wait until the peer#2 see the right size of 'big.bin'.
   timer.start();
   while (this->fileManagers[1]->getAmount() < NUMBER_OF_CHUNK * CHUNK_SIZE)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 10000)
         QFAIL("After adding the big file 'big.bin' the amount of data must be greater the 32KiB");
   }

   // Ask the ashes of "big.bin" from the first peer to the second one.
   Protos::Common::Entry fileEntry;
   fileEntry.set_type(Protos::Common::Entry_Type_FILE);
   fileEntry.set_path("/");
   fileEntry.set_name("big.bin");
   fileEntry.set_size(0); // No obligation to set the correct size.
   for (quint32 i = 0; i < NUMBER_OF_CHUNK; i++)
      fileEntry.add_chunk();
   // Sets the root directory.
   fileEntry.mutable_shared_dir()->CopyFrom(this->resultListener.getEntriesResultList().first().result(0).entries().entry(0).shared_dir());

   QSharedPointer<IGetHashesResult> result = this->peerManagers[0]->getPeers()[0]->getHashes(fileEntry);
   QVERIFY(!result.isNull());
   connect(result.data(), &IGetHashesResult::result, &this->resultListener, &ResultListener::hashesResult);
   connect(result.data(), &IGetHashesResult::nextHash, &this->resultListener, &ResultListener::nextHashResult);
   result->start();

   timer.start();
   while (this->resultListener.getLastGetHashesResult().status() != Protos::Core::GetHashesResult::OK && this->resultListener.getLastGetHashesResult().nb_hash() != NUMBER_OF_CHUNK)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 5000)
         QFAIL("We don't receive the correct receive after asking for hashes");
   }

   // Wait to have all the hashes.
   timer.start();
   while (this->resultListener.getNbHashReceivedFromLastGetHashes() != NUMBER_OF_CHUNK)
   {
      QTest::qWait(100);
      if (timer.elapsed() > 30000)
         QFAIL("We don't receive all the hashes");
   }
}

void Tests::askForAChunk()
{
   qDebug() << "===== askForAChunk() =====";

   connect(this->peerManagers[1].data(), &IPeerManager::getChunk, &this->resultListener, &ResultListener::getChunk);

   Protos::Core::GetChunk getChunkMessage;
   getChunkMessage.mutable_chunk()->set_hash(this->resultListener.getLastReceivedHash().getData(), Common::Hash::HASH_SIZE);
   getChunkMessage.set_offset(0);
   QSharedPointer<IGetChunkResult> result = this->peerManagers[0]->getPeers()[0]->getChunk(getChunkMessage);
   QVERIFY(!result.isNull());
   connect(result.data(), &IGetChunkResult::result, &this->resultListener, &ResultListener::chunkResult);
   connect(result.data(), &IGetChunkResult::stream, &this->resultListener, &ResultListener::stream);
   result->start();

   QElapsedTimer timer;
   timer.start();
   while (!this->resultListener.isStreamReceived())
   {
      QTest::qWait(100);
      if (timer.elapsed() > 10000)
         QFAIL("We don't receive the stream");
   }
}

void Tests::cleanupTestCase()
{
   qDebug() << "===== cleanupTestCase() =====";

   for (QListIterator<TestServer*> i(this->servers); i.hasNext();)
      delete i.next();

   delete this->peerUpdater;
}

bool Tests::createInitialFiles()
{
   qDebug() << "Create the directories structure in" << QDir::currentPath();

   if (!this->deleteAllFiles())
      return false;

   return
      Common::Global::createFile("sharedDirs/peer1/subdir/a.txt") &&
      Common::Global::createFile("sharedDirs/peer1/subdir/b.txt") &&
      Common::Global::createFile("sharedDirs/peer1/subdir/c.txt") &&
      Common::Global::createFile("sharedDirs/peer1/d.txt") &&
      Common::Global::createFile("sharedDirs/peer1/e.txt") &&

      Common::Global::createFile("sharedDirs/peer2/subdir/f.txt") &&
      Common::Global::createFile("sharedDirs/peer2/subdir/g.txt") &&
      Common::Global::createFile("sharedDirs/peer2/subdir/h.txt") &&
      Common::Global::createFile("sharedDirs/peer2/i.txt") &&
      Common::Global::createFile("sharedDirs/peer2/j.txt") &&
      Common::Global::createFile("sharedDirs/peer2/k.txt");
}

bool Tests::deleteAllFiles()
{
   return Common::Global::recursiveDeleteDirectory("sharedDirs");
}

