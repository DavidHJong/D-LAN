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

#include <QTimer>
#include <QThread>
#include <QWaitCondition>
#include <QWeakPointer>
#include <QMutex>

namespace Common
{
   class Thread;
   class IRunnable;

   class ThreadPool : public QObject
   {
      Q_OBJECT

   public:
      ThreadPool(int nbMinThread, int threadInactiveLifetime = 60000);
      ~ThreadPool();

      void setStackSize(uint stackSize);

      void run(QWeakPointer<IRunnable> runnable);
      void wait(QWeakPointer<IRunnable> runnable);

   private slots:
      void runnableFinished();
      void threadTimeout();

   private:
      const int nbMinThread;
      const int threadInactiveLifetime;
      uint stackSize;

      QList<Thread*> activeThreads;
      QList<Thread*> inactiveThreads;
   };

   class Thread : public QThread
   {
      Q_OBJECT
   public:
      Thread(int lifetime, uint stackSize = 0);
      ~Thread();
      void setRunnable(QWeakPointer<IRunnable> runnable);
      void waitRunnableFinished();
      void startTimer();
      QWeakPointer<IRunnable> getRunnable() const;

   signals:
      void timeout();
      void runnableFinished();

   protected:
      void run();

   private:
      QWeakPointer<IRunnable> runnable;
      QTimer timer;

      mutable QWaitCondition waitCondition;
      mutable QMutex mutex;

      bool toStop;
      bool active;
   };
}
