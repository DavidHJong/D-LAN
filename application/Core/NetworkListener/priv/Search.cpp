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
  
#include <priv/Search.h>
using namespace NL;

#include <QRandomGenerator64>

#include <Common/ProtoHelper.h>
#include <Common/Network/MessageHeader.h>
#include <Common/Settings.h>

#include <priv/Log.h>

/**
  * @class NL::Search
  * @author mcuony
  * @author gburri
  */

Search::Search(UDPListener& uDPListener) :
   uDPListener(uDPListener), nbResult(0), tag(0)
{
}

quint64 Search::search(const Protos::Common::FindPattern& findPattern)
{
   if (this->tag != 0)
   {
      L_ERRO(QString("You can't launch a search twice!"));
      return 0;
   }

   this->nbResult = 0;
   this->timer.start();

   Protos::Core::Find findMessage;

   this->tag = QRandomGenerator64::global()->generate64();
   findMessage.set_tag(this->tag);
   findMessage.mutable_pattern()->CopyFrom(findPattern);

   connect(&this->uDPListener, &UDPListener::newFindResultMessage, this, &Search::newFindResult);

   this->uDPListener.send(Common::MessageHeader::CORE_FIND, findMessage);

   return this->tag;
}

qint64 Search::elapsed()
{
   return timer.elapsed();
}

/**
  * Called when a result is recevied, if the tag matches, we forward the result to our listeners.
  */
void Search::newFindResult(const Protos::Common::FindResult& result)
{
   if (result.tag() == this->tag && this->nbResult + static_cast<quint32>(result.entry_size()) <= SETTINGS.get<quint32>("max_number_of_result_shown"))
   {
      this->nbResult += result.entry_size();
      emit found(result);
   }
}
