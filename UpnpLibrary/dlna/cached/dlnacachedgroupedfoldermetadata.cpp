#include "dlnacachedgroupedfoldermetadata.h"

DlnaCachedGroupedFolderMetaData::DlnaCachedGroupedFolderMetaData(MediaLibrary *library, QString name, QObject *parent):
    DlnaStorageFolder(parent),
    library(library),
    name(name),
    children()
{
}

void DlnaCachedGroupedFolderMetaData::addFolder(QString stringQuery,
                                                QString stringQueryForChild,
                                                QString name)
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
    else
        return 0;
}

void DlnaCachedGroupedFolderMetaData::setNetworkAccessManager(QNetworkAccessManager *manager)
{
    foreach (DlnaCachedFolderMetaData *child, children)
        child->setNetworkAccessManager(manager);
}
