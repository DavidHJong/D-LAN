#ifndef FILEMANAGER_FILEUPDATER_H
#define FILEMANAGER_FILEUPDATER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QString>
#include <QLinkedList>

#include <priv/FileUpdater/DirWatcher.h>

namespace FileManager
{
   class FileManager;
   class DirWatcher;
   class SharedDirectory;
   class Directory;
   class File;

   class FileUpdater : public QThread
   {
      static const int minimumDurationWhenHashing = 30; ///< In seconds.

   public:
      FileUpdater(FileManager* fileManager);
      void addRoot(SharedDirectory* dir);
      void rmRoot(SharedDirectory* dir);

      void run();

   private:
      void createNewFile(Directory* dir, const QString& filename, qint64 size);

      /**
        * It will take some file from 'fileWithoutHashes' and compute theirs hashes.
        * The duration of the compuation is minimum 'minimumDurationWhenHashing'.
        */
      void computeSomeHashes();

      /**
        * Scan recursively all the directories and files contained
        * in dir. Create the associated cached tree structure under
        * given 'SharedDirectory'.
        */
      void scan(SharedDirectory* dir);

      void treatEvents(const QList<WatcherEvent>& events);

      FileManager* fileManager;
      DirWatcher* dirWatcher;

      QWaitCondition dirNotEmpty;
      QMutex mutex;

      QLinkedList<SharedDirectory*> dirsToScan; ///< When a new shared directory is added, it is put in this list until it is scanned.

      QLinkedList<File*> fileWithoutHashes;
   };
}
#endif
