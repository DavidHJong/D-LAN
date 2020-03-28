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

#include <QString>

#include <Common/Hash.h>
#include <priv/Cache/Directory.h>

namespace FM
{
   class Cache;
   class FileManager;

   class SharedDirectory : public Directory
   {
   public:
      SharedDirectory(Cache* cache, const QString& path, const Common::Hash& id);
      SharedDirectory(Cache* cache, const QString& path);

      void mergeSubSharedDirectories();

      void populateEntry(Protos::Common::Entry* entry, bool setSharedDir = false) const;

   private:
      void init();

   public:
      ~SharedDirectory();
      void del(bool invokeDelete = true);

      void moveInto(Directory* directory);
      void moveInto(const QString& path);

      QString getPath() const;

      /**
        * Return the full path to the shared directory.
        * There is always a slash at the end.
        * For exemple :
        *  - '/home/paul/movies/'
        *  - '/'.
        *  - 'C:/Users/Paul/My Movies/'
        *  - 'G:/'
        */
      QString getFullPath() const;

      SharedDirectory* getRoot() const;

      Common::Hash getId() const;

   private:
      static QString dirName(const QString& path);
      QString pathWithoutDirName(const QString& path);

      QString path; // Always ended by a slash '/'.
      Common::Hash id;
   };
}
