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

namespace DM
{
   const int RETRY_PEER_GET_HASHES_PERIOD = 10000; // [ms]. If the hashes cannot be retrieve frome a peer, we wait 10s before retrying.
   const int RETRY_GET_ENTRIES_PERIOD = 10000; // [ms]. If a directory can't be browsed, we wait 10s before retrying.
   const int RESTART_DOWNLOADS_PERIOD_IF_ERROR = 10000; // [ms]. If one or more download has a status >= 0x20 then it will be restarted periodically.

   // 2 -> 3 : BLAKE -> Sha-1
   // 3 -> 4 : Replace Entry::complete by a status.
   const int FILE_QUEUE_VERSION = 4;
}
