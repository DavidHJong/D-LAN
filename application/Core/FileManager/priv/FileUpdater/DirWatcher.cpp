#include <priv/FileUpdater/DirWatcher.h>
using namespace FM;

#include <QtCore/QtDebug>

#include <priv/Log.h>

#if defined(Q_OS_WIN32)
   #include <priv/FileUpdater/DirWatcherWin.h>
#endif

DirWatcher* DirWatcher::getNewWatcher()
{
#if defined(Q_OS_WIN32)
   return new DirWatcherWin();
#else
   LOG_WARN("Cannot create a watcher for the current platform, no implementation.");
   return 0;
#endif
}

WatcherEvent::WatcherEvent()
   : type(WatcherEvent::UNKNOWN)
{}

 WatcherEvent::WatcherEvent(const WatcherEvent& e)
   : type(e.type), path1(e.path1), path2(e.path2)
 {}

WatcherEvent::WatcherEvent(Type type)
   : type(type)
{}

WatcherEvent::WatcherEvent(WatcherEvent::Type type, const QString& path1)
      : type(type), path1(path1)
{}

WatcherEvent::WatcherEvent(WatcherEvent::Type type, const QString& path1, const QString& path2)
      : type(type), path1(path1), path2(path2)
{}

QString WatcherEvent::toStr()
{
   QString str;
   switch (this->type)
   {
   case RENAME_DIR : str += "RENAME_DIR"; break;
   case RENAME_FILE : str += "RENAME_FILE"; break;
   case NEW_FILE : str += "NEW_FILE"; break;
   case DELETE_FILE : str += "DELETE_FILE"; break;
   case CONTENT_FILE_CHANGED : str += "CONTENT_FILE_CHANGED"; break;
   case TIMEOUT : str += "TIMEOUT"; break;
   case UNKNOWN : str += "UNKNOWN"; break;
   default : str += "UNKNOWN"; break;
   }
   str += " :\n";
   if (!this->path1.isEmpty())
      str.append("  ").append(this->path1);
   if (!this->path1.isEmpty() && !this->path2.isEmpty())
      str.append("\n");
   if (!this->path2.isEmpty())
      str.append("  ").append(this->path2);
   return str;
}
