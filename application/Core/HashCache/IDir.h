#pragma once

#include <QList>

#include <IEntry.h>

namespace HC
{
   class IFile;
   class IDir : public IEntry
   {
   public:
      virtual ~IDir() {}

      virtual QList<IFile> getFiles() = 0;
      virtual QList<IDir> getDirs() = 0;
   };
}
