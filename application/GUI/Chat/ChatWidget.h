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

#include <QWidget>
#include <QTextDocument>
#include <QStyledItemDelegate>
#include <QTextCharFormat>
#include <QUrl>

#include <Common/Containers/SortedList.h>
#include <Common/RemoteCoreController/ICoreConnection.h>

#include <Peers/PeerListModel.h>
#include <Chat/ChatModel.h>
#include <MDI/MdiWidget.h>
#include <Emoticons/Emoticons.h>
#include <Emoticons/EmoticonsWidget.h>
#include <AutoComplete/AutoComplete.h>

namespace Ui {
  class ChatWidget;
}

namespace GUI
{
   class PeerListChatDelegate : public QStyledItemDelegate
   {
   public:
      void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      //QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
   };

   class ChatDelegate : public QStyledItemDelegate
   {
   public:
      ChatDelegate(QTextDocument& textDocument);
      void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      QSize	sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
//      QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
//      void setEditorData(QWidget* editor, const QModelIndex& index) const;

   private:
      QTextDocument& textDocument;
   };

   class ChatWidget : public MdiWidget
   {
      static const int DEFAULT_FONT_SIZE;

      Q_OBJECT
   public:
      explicit ChatWidget(QSharedPointer<RCC::ICoreConnection> coreConnection, Emoticons& emoticons, QWidget* parent = nullptr);
      explicit ChatWidget(QSharedPointer<RCC::ICoreConnection> coreConnection, Emoticons& emoticons, const QString& roomName, QWidget* parent = nullptr);
      ~ChatWidget();

      bool isGeneral() const;
      QString getRoomName() const;

   signals:
      void browsePeer(const Common::Hash& peerID);

   private slots:
      void sendMessage();
      void newRows(const QModelIndex& parent, int start, int end);
      void sendMessageStatus(ChatModel::SendMessageStatus status);
      void scrollChanged(int value);

      void displayContextMenuPeers(const QPoint& point);
      void browseSelectedPeers();
      void copyIPToClipboard();

      void displayContextMenu(const QPoint& point);
      void copySelectedLineToClipboard();
      void browseSelectedMessages();

      void currentCharFormatChanged(const QTextCharFormat& charFormat);
      void cursorPositionChanged();
      void textChanged();      
      void documentChanged(int position, int charsRemoved, int charsAdded);

      void setFocusTxtMessage();

      void comboFontSizeChanged(int index);
      void setBold(bool toggled);
      void setItalic(bool toggled);
      void setUnderline(bool toggled);
      void setTextColor(QColor color);

      void resetFormat();

      void emoticonsButtonToggled(bool);
      void messageWordTyped(int position, const QString& word);
      void emoticonsWindowHidden();
      void emoticonsWindowHiddenDelayed();
      void insertEmoticon(const QString& theme, const QString& emoticonName);
      void defaultEmoticonThemeChanged(const QString& theme);

      void autoCompleteStringAdded(QString str);
      void autoCompleteLastCharRemoved();
      void autoCompleteClosed();

   protected:
      void keyPressEvent(QKeyEvent* keyEvent) override;
      void changeEvent(QEvent* event) override;
      bool eventFilter(QObject* obj, QEvent* event) override;

   private:
      void init();
      void applyCurrentFormat();
      void connectFormatWidgets();
      void disconnectFormatWidgets();
      void setComboFontSize(int fontSize);
      void displayEmoticons(const QPoint& positionSender, const QSize& sizeSender);

      void activatePeerNameInsertionMode();

      QList<Common::Hash> getPeerAnswers() const;

      void onActivate() override;

      void setNewMessageState(bool newMessage);

      static QUrl buildUrlEmoticon(const QString& theme, const QString& emoticonName);
      static QString htmlEmoticon(const QString& theme, const QString& emoticonName);

      Ui::ChatWidget* ui;
      EmoticonsWidget* emoticonsWidget;
      QTextDocument textDocument;

      // Current peers answered.
      struct Answer {
         int begin;
         int end;
         bool startWithSpace;
         Common::Hash peerID;
         bool operator<(const Answer& other) const { return this->begin < other.begin; }
         bool operator==(const Answer& other) const { return this->begin == other.begin; }
      };
      Common::SortedList<Answer> answers;
      bool peerNameInsertionMode;
      Answer currentAnswer;
      AutoComplete* autoComplete;

      QSharedPointer<RCC::ICoreConnection> coreConnection;
      Emoticons& emoticons;
      PeerListModel peerListModel;
      PeerListChatDelegate peerListDelegate;
      ChatModel chatModel;
      ChatDelegate chatDelegate;

      bool autoScroll;
   };
}
