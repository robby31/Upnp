#include "dlnacachedgroupedfoldermetadata.h"

qint64 DlnaCachedGroupedFolderMetaData::objectCounter = 0;

DlnaCachedGroupedFolderMetaData::DlnaCachedGroupedFolderMetaData(MediaLibrary *library, const QString& name, QObject *parent):
    DlnaStorageFolder(parent),
    library(library),
    name(name)
{
    ++objectCounter;
}

DlnaCachedGroupedFolderMetaData::~DlnaCachedGroupedFolderMetaData()
{
    --objectCounter;
}

void DlnaCachedGroupedFolderMetaData::addFolder(const QString &stringQuery,
                                                const QString& stringQueryForChild,
                                                const QString& name)
{
    DlnaCachedFolderMetaData* child;

    child = new DlnaCachedFolderMetaData(library,
                                         stringQuery, stringQueryForChild,
                                         name,
                                         this);
    child->setId(QString("%1").arg(children.length()+1));
    children.append(child);
    child->setDlnaParent(this);
}

DlnaResource *DlnaCachedGroupedFolderMetaData::getChild(int index, QObject *parent)
{
    Q_UNUSED(parent)

    if (index>=0 && index<children.count())
        return children.at(index);

    return Q_NULLPTR;
}
