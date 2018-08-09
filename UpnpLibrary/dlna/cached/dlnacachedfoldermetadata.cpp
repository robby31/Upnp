#include "dlnacachedfoldermetadata.h"

DlnaCachedFolderMetaData::DlnaCachedFolderMetaData(MediaLibrary *library, const QString& stringQuery, QString stringQueryForChild, QString name, QObject *parent):
    DlnaStorageFolder(parent),
    library(library),
    m_name(name),
    query(stringQuery, library ? library->database() : QSqlDatabase()),
    stringQueryForChild(stringQueryForChild),
    nbChildren(-1),
    m_nam(Q_NULLPTR)
{
    if (query.isSelect() && query.isActive()) {
        if (query.last())
            nbChildren = query.at() + 1;
        else
            nbChildren = 0;
    }
}

DlnaResource *DlnaCachedFolderMetaData::getChild(int index, QObject *parent)
{
    DlnaCachedFolder *child = Q_NULLPTR;

    if (library && query.seek(index)) {
        QString name = query.value(0).toString();
        if (query.record().count()==2)
            name = query.value(1).toString();

        QString childName = name;
        QString childQuery;

        // metaData is null ?
        if (query.value(0).isNull())
        {
            childName = QString("No %1").arg(m_name);
            childQuery = stringQueryForChild.arg(" is null");
        }
        else
        {
            QString param = QString("=\"%1\"").arg(query.value(0).toString());
            childQuery = stringQueryForChild.arg(param);
        }

        child = new DlnaCachedFolder(library,
                                     QSqlQuery(childQuery,  library->database()),
                                     childName,
                                     false, -1,
                                     parent != Q_NULLPTR ? parent : this);
    }

    if (child)
    {
        child->setId(QString("%1").arg(index+1));
        child->setNetworkAccessManager(m_nam);
        child->setDlnaParent(this);
    }

    return child;
}
