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

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QString>
#include <QList>
#include <QElapsedTimer>

#include <priv/FileUpdater/DirWatcher.h>
#include <priv/Cache/FileHasher.h>

namespace FM
{
   class FileManager;
   class SharedEntry;
   class Directory;
   class File;
   class Entry;
   class WaitCondition;

   class FileUpdater : public QThread
   {
      Q_OBJECT

   public:
      FileUpdater(FileManager* fileManager);
      ~FileUpdater();

      void stop();
      void prioritizeAFileToHash(File* file);

      bool isScanning() const;
      bool isHashing() const;
      int getProgress() const;

      void addRoot(SharedEntry* sharedEntry);
      void rmRoot(SharedEntry* sharedEntry, Directory* dir2 = nullptr);

   signals:
      void fileCacheLoaded();
      void deleteSharedEntry(SharedEntry*);

   protected:
      void run();

   private:
      void computeSomeHashes();
      void updateHashingProgress();

      void stopHashing();

      void scan(Entry* entry, bool addUnfinished = false);
      // void addScannedFile(const FileInfo& fileInfo, File* fileCache = nullptr); TODO: To remove

      void stopScanning(Entry* entry = nullptr);

      void deleteEntry(Entry* entry);
      void removeFromEntriesToScan(Entry* entry);
      void removeFromFilesWithoutHashes(Entry* entry);

      //void restoreFromFileCache(SharedDirectory* dir);

      bool processEvents(const QList<WatcherEvent>& events);

      const int SCAN_PERIOD_UNWATCHABLE_DIRS;

      FileManager* fileManager;
      DirWatcher* dirWatcher;

      /*class FileCacheInformation
      {
      public:
         FileCacheInformation(const Protos::FileCache::Hashes* fileCache);
         ~FileCacheInformation();

         void newFile();
         const Protos::FileCache::Hashes* getFileCache();
         int getProgress() const;

      private:
         void computeFileCacheNbFiles(const Protos::FileCache::Hashes::Dir& dir);

         const Protos::FileCache::Hashes* fileCache; ///< The hashes from the saved file cache. Used only temporally at the beginning of 'run()'.
         int fileCacheNbFiles;
         int fileCacheNbFilesLoaded;
      };
      FileCacheInformation* fileCacheInformation; // Only used during the loading of 'fileCache'.*/

      bool toStop; ///< Set to true when the service must be stopped.

      int progress;

      WaitCondition* dirEvent; ///< Using to wait when a sharing directory is added or deleted.
      mutable QMutex mutex; ///< Prevent the access from many thread to the internal data like 'filesWithoutHashes' for example.

      QList<Entry*> unwatchableEntries;
      QElapsedTimer timerScanUnwatchable;
      QList<Entry*> entriesToScan; ///< When something change in a directory or in a file we put it in this list until it is scanned.
      Entry* currentScanningEntry;
      QWaitCondition scanningStopped;
      mutable QMutex scanningMutex;

      mutable QMutex hashingMutex;
      bool toStopHashing;
      FileHasher fileHasher;

      QList<Entry*> entriesToRemove;

      QList<File*> filesWithoutHashes;
      QList<File*> filesWithoutHashesPrioritized;
      qint64 remainingSizeToHash;
   };
}
