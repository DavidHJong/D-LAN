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

namespace FM
{
   /**
     * To write data into a chunk.
     */
   class IDataWriter
   {
   public:
      virtual ~IDataWriter() {}

      /**
        * @exception IOErrorException
        * @exception ChunkDeletedException When trying to write to a deleted chunk.
        * @exception TryToWriteBeyondTheEndOfChunkException
        * @exception hashMissmatchException This occurs only when the setting 'check_received_data_integrity' is enabled. When this exception is thrown the chunk data are reset.
        */
      virtual bool write(const char* buffer, int nbBytes) = 0;
   };
}
