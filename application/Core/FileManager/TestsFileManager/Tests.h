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
  
#ifndef TESTS_FILEMANAGER_H
#define TESTS_FILEMANAGER_H

#include <QTest>
#include <QDir>

#include <Protos/common.pb.h>

#include <Builder.h>
#include <IFileManager.h>
using namespace FM;

class Tests : public QObject
{
   struct FindResult : public QMap<int, QList<QString>> {};

   Q_OBJECT
public:
   Tests();

private slots:
   void initTestCase();

   void testWordIndex();

   void createFileManager();

   /***** Adding shared directories *****/
   void addASharedDirectoryIncoming();
   void addASharedDirectory();
   void addAnAlreadySharedDirectory();
   void swapTwoDirectories();
   void addInexistingSharedDirectory();
   void addSubSharedDirectories();
   void addSuperSharedDirectories();

   /***** Modification of the file system *****/
   void createAFile();
   void moveAFile();
   void renameAFile();
   void modifyAFile();
   void removeAFile();
   void createASubFile();
   void createABigFile();
   void modifyABigFile();
   void removeABigFile();
   void createADirectory();
   void renameADirectory();
   void moveAnEmptyDirectory();
   void moveADirectoryContainingFiles();
   void removeADirectory();
   void createAnEmptyFile();

   /***** Ask for chunks by hash *****/
   void getAnExistingChunk();
   void getANonExistingChunk();

   /***** Get Hashes from a FileEntry which the hash is already computed *****/
   void getHashesFromAFileEntry1();

   /***** Get Hashes from a FileEntry which the hash is unknown *****/
   void getHashesFromAFileEntry2();

   /***** Browse the shared directories *****/
   void browseSomeDirectories();

   /***** Find files and directories by keywords *****/
   void findExistingFilesWithOneWord();
   void findNonExistingFilesWithOneWord();
   void findFilesWithSomeWords1();
   void findFilesWithSomeWords2();
   void findFilesWithResultFragmentation();
   void findFilesWithSomeWordsAndExtensions();
   void findFilesWithSomeWordsAndExtensionsAndSizeRange();
   void findFilesByExtensions();
   void findFilesByExtensionsAndSizeRange();
   void findFilesBySizeRange();

   /***** Ask if the given hashes are known *****/
   void haveChunks();

   /***** Ask for the amount of shared byte *****/
   void printAmount();

   /***** Removing shared directories *****/
   void rmSharedDirectory();

   /********** Unit tests of internals classes **********/

   /***** Speed test of the class 'Chunks' *****/
   void chunksPerformance();

   /***** The extension index class *****/
   void extensionIndexAddItem();
   void extensionIndexRmItem();
   void extensionIndexChangeItem();
   void extensionIndexSearchWithOneExtension();
   void extensionIndexSearchWithSomeExtensions();

   void cleanupTestCase();

private:
   void createInitialFiles();
   void deleteAllFiles();

   void printSearch(const QString& terms, const Protos::Common::FindResult& result);
      void compareExpectedResult(const Protos::Common::FindResult& result, const FindResult& expectedResult);

   void addSuperSharedDirectoriesAndMerge();

   static void compareStrRegexp(const QString& regexp, const QString& str);

   QStringList sharedDirs;
   QSharedPointer<IFileManager> fileManager;
};

#endif
