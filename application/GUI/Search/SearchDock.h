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
#include <QDockWidget>
#include <QSharedPointer>

#include <Protos/common.pb.h>

#include <Common/KnownExtensions.h>
#include <Common/RemoteCoreController/ICoreConnection.h>

#include <Search/SearchUtils.h>

namespace Ui {
   class SearchDock;
}

namespace GUI
{
   class SearchDock : public QDockWidget
   {
      Q_OBJECT

   public:
      explicit SearchDock(QSharedPointer<RCC::ICoreConnection> coreConnection, QWidget* parent = 0);
      ~SearchDock();

      void setFocusToLineEdit();

   signals:
      void search(const Protos::Common::FindPattern&, bool local);

   protected:
      void changeEvent(QEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;

   private slots:
      void coreConnected();
      void coreDisconnected(bool force);

      void advancedOptionsVisibility(bool shown);
      void search();
      void clear();

      void saveSettings();

   private:
      void loadSettings();
      void updateComboTypes();

      SearchType currentType() const;
      quint64 currentMinSize();
      quint64 currentMaxSize();

      Ui::SearchDock* ui;

      QSharedPointer<RCC::ICoreConnection> coreConnection;
   };
}
