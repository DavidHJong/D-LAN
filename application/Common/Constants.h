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

#include <QDir>

namespace Common
{
   class Constants
   {
   public:      
      // 2 -> 3 : BLAKE -> Sha-1
      // 3 -> 4 : New chat protocol + changes of the 'GET_ENTRIES_RESULT' message.
      static const quint32 PROTOCOL_VERSION;

      static const quint16 DEFAULT_CORE_REMOTE_CONTROL_PORT;

      static const QString APPLICATION_FOLDER_NAME;

      static const QString FILE_EXTENSION;

      static const QString FILE_CACHE;
      static const QString FILE_QUEUE;
      static const QString DIR_CHAT_MESSAGES;
      static const QString FILE_CHAT_MESSAGES;
      static const QString FILE_CHAT_ROOM_MESSAGES;

      static const QString CORE_SETTINGS_FILENAME;
      static const QString GUI_SETTINGS_FILENAME;

      static const QString LANGUAGE_DIRECTORY;

      static const QString STYLE_DIRECTORY;
      static const QString STYLE_FILE_NAME;

      static const QString SERVICE_NAME;

      static const int PROTOBUF_STREAMING_BUFFER_SIZE;

      static const QString BINARY_PREFIXS[];

      static const int MAX_NB_HASHES_PER_ENTRY_GUI_BROWSE;
   };
}
