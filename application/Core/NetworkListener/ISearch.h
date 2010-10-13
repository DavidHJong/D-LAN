#ifndef NETWORKLISTENER_ISEARCH_H
#define NETWORKLISTENER_ISEARCH_H

#include <QObject>
#include <QString>

#include <Protos/common.pb.h>

namespace NL
{
   class ISearch : public QObject
   {
      Q_OBJECT
   public:
      virtual ~ISearch() {}
      virtual void search(const QString& words) = 0;

   signals:
      void found(const Protos::Common::FindResult& result);
   };
}
#endif
