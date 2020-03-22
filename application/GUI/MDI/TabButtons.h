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

#include <QAbstractButton>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

#include <functional>
using namespace std;

namespace GUI
{
   class TabButton : public QAbstractButton
   {
      Q_OBJECT
   public:
      TabButton(QWidget* parent = nullptr);
      virtual QSize sizeHint() const;
      virtual QSize minimumSizeHint() const;
      virtual void enterEvent(QEvent *event);
      virtual void leaveEvent(QEvent *event);
      virtual void paintEvent(QPaintEvent *event);

   protected:
      virtual void drawPrimitive(const QStyleOption& opt, QPainter& p) = 0;
   };

/////

   class TabCloseButton : public TabButton
   {
       Q_OBJECT
   public:
       TabCloseButton(QWidget* widget, QWidget* parent = nullptr, bool autoDelete = true, function<QString()> tooltipFun = nullptr);

   protected:
       void changeEvent(QEvent* event);
       void drawPrimitive(const QStyleOption& opt, QPainter& p);

   signals:
      void clicked(QWidget* widget);

   private slots:
      void buttonClicked();

   private:
      void setToolTipTranslate();
      QWidget* widget;
      bool autoDelete;
      function<QString()> tooltipFun;
   };

/////

   class TabRefreshButton : public TabButton
   {
       Q_OBJECT
   public:
       TabRefreshButton(QWidget* parent = nullptr);

   protected:
       void changeEvent(QEvent* event);
       void drawPrimitive(const QStyleOption& option, QPainter& painter);

   private:
       void setToolTipTranslate();
       QIcon icon;
   };
}
