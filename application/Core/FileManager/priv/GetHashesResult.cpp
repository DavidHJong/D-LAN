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

#include <priv/GetHashesResult.h>

#include <QSharedPointer>
#include <QList>
#include <QMutexLocker>
#include <QMetaType>

#include <Protos/core_protocol.pb.h>

#include <priv/Cache/File.h>
#include <priv/Cache/Chunk.h>
#include <priv/Log.h>

using namespace FM;

GetHashesResult::GetHashesResult(const Protos::Common::Entry& fileEntry, Cache& cache, FileUpdater& fileUpdater) :
   fileEntry(fileEntry), file(nullptr), cache(cache), fileUpdater(fileUpdater)
{
   qRegisterMetaType<Protos::Core::HashResult>("Protos::Core::HashResult");

   L_DEBU("GetHashesResult::GetHashesResult(..)");
}

GetHashesResult::~GetHashesResult()
{
   // After the 'emit nextHash(chunk->getHash());' the receiver (in an other thread) can decide to clear the QSharedPointer, if it does and it's the last reference the
   // object will be destroyed by an another thread and 'mutex' will be unlock by this other thread . . .
   QMutexLocker locker(&this->mutex);

   L_DEBU("GetHashesResult::~GetHashesResult()");

   disconnect(&this->cache, &Cache::chunkHashKnown, this, &GetHashesResult::chunkHashKnown);
}

/**
  * Called from the main thread.
  */
Protos::Core::GetHashesResult GetHashesResult::start()
{
   Protos::Core::GetHashesResult result;

   if (!(this->file = this->cache.getFile(this->fileEntry)))
   {
      result.set_status(Protos::Core::GetHashesResult_Status_DONT_HAVE);
      return result;
   }

   const QVector<QSharedPointer<Chunk>>& chunks = this->file->getChunks();
   if (this->fileEntry.chunk_size() != chunks.size())
   {
      L_ERRO("The number of chunks of the given file entry doesn't match the cache file number.");
      result.set_status(Protos::Core::GetHashesResult_Status_ERROR_UNKNOWN);
      return result;
   }

   {
      QMutexLocker locker(&this->mutex);

      connect(&this->cache, &Cache::chunkHashKnown, this, &GetHashesResult::chunkHashKnown, Qt::DirectConnection);

      int nbOfHashToBeSent = 0;
      int j = 0;
      for (QVectorIterator<QSharedPointer<Chunk>> i(chunks); i.hasNext();)
      {
         auto chunk = i.next();
         const Protos::Common::Hash& protoChunk = this->fileEntry.chunk(j++);

         if (protoChunk.hash().size() > 0)
         {
            nbOfHashToBeSent++;
            if (chunk->hasHash())
               this->sendNextHash(chunk, true);
            else
               this->hashesRemaining << chunk->getNum();
         }
      }

      result.set_nb_hash(nbOfHashToBeSent);
   }

   result.set_status(Protos::Core::GetHashesResult_Status_OK);

   // No hash to send.
   if (this->hashesRemaining.isEmpty())
      disconnect(&this->cache, &Cache::chunkHashKnown, this, &GetHashesResult::chunkHashKnown);
   else
      // If at least one hash is missing we tell the file updater to compute the remaining ones.
      this->fileUpdater.prioritizeAFileToHash(this->file);

   return result;
}

void GetHashesResult::chunkHashKnown(QSharedPointer<Chunk> chunk)
{
   if (chunk->isOwnedBy(this->file))
   {
      QMutexLocker locker(&this->mutex);
      this->sendNextHash(chunk, false);
   }
}

void GetHashesResult::sendNextHash(QSharedPointer<Chunk> chunk, bool direct)
{
   if (!direct)
   {
      int i = this->hashesRemaining.indexOf(chunk->getNum());
      if (i != -1)
      {
         this->hashesRemaining.removeAt(i);
         if (this->hashesRemaining.empty())
            disconnect(&this->cache, &Cache::chunkHashKnown, this, &GetHashesResult::chunkHashKnown);
      }
   }

   Protos::Core::HashResult hashResult;
   hashResult.set_num(chunk->getNum());
   hashResult.mutable_hash()->set_hash(chunk->getHash().getData(), Common::Hash::HASH_SIZE);
   emit nextHash(hashResult);
}
