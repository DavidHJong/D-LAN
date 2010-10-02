#include <priv/GetHashesResult.h>

#include <QSharedPointer>
#include <QList>
#include <QMetaType>

#include <priv/Cache/File.h>
#include <priv/Cache/Chunk.h>
#include <priv/Log.h>

using namespace FM;

GetHashesResult::GetHashesResult(const Protos::Common::Entry& fileEntry, Cache& cache, FileUpdater& fileUpdater)
   : fileEntry(fileEntry), file(0), cache(cache), fileUpdater(fileUpdater), nbHash(0)
{
   qRegisterMetaType<Common::Hash>("Common::Hash");
}

Protos::Core::GetHashesResult GetHashesResult::start()
{
   Protos::Core::GetHashesResult result;

   this->file = this->cache.getFile(this->fileEntry);
   QList< QSharedPointer<Chunk> > chunks = this->cache.getChunks(this->fileEntry);

   if (chunks.isEmpty())
   {
      result.set_status(Protos::Core::GetHashesResult_Status_DONT_HAVE);
      return result;
   }

   this->nbHash = chunks.size();
   connect(&this->cache, SIGNAL(chunkHashKnown(QSharedPointer<Chunk>)), this, SLOT(chunkHashKnown(QSharedPointer<Chunk>)), Qt::DirectConnection);

   result.set_nb_hash(chunks.size());
   for (QListIterator< QSharedPointer<Chunk> > i(chunks); i.hasNext();)
   {
      QSharedPointer<Chunk> chunk(i.next());
      if (chunk->hasHash())
      {
         this->decNbHash();
         emit nextHash(chunk->getHash());
      }
      else // If only one hash is missing we tell the FileUpdater to compute the remaining ones.
      {
         this->fileUpdater.prioritizeAFileToHash(this->file);
         break;
      }
   }

   result.set_status(Protos::Core::GetHashesResult_Status_OK);
   return result;
}

void GetHashesResult::chunkHashKnown(QSharedPointer<Chunk> chunk)
{
   if (chunk->isOwnedBy(this->file))
   {
      this->decNbHash();
      emit nextHash(chunk->getHash());
   }
}
void GetHashesResult::decNbHash()
{
   if (!--this->nbHash)
      disconnect(&this->cache, SIGNAL(chunkHashKnown(QSharedPointer<Chunk>)), this, SLOT(chunkHashKnown(QSharedPointer<Chunk>)));
}
