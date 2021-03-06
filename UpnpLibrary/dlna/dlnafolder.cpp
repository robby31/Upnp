#include "dlnafolder.h"

DlnaFolder::DlnaFolder(const QString &filename, QObject *parent):
    DlnaStorageFolder(parent),
    fileinfo(filename)
{
    QDir folder(fileinfo.absoluteFilePath());
    QStringList filter;
    filter << "*";
    foreach (QString fd, folder.entryList(filter,
                                          QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                                          QDir::DirsFirst | QDir::Name)) {
        QFileInfo new_file(folder, fd);

        QMimeDatabase mimeDb;

        if (new_file.isDir()) {
            children.append(new_file);
        }
        else if (mimeDb.mimeTypeForFile(new_file).name().startsWith("audio/")) {
            children.append(new_file);
        }
        else if (mimeDb.mimeTypeForFile(new_file).name().startsWith("video/")) {
            children.append(new_file);
        }
    }
}

DlnaResource *DlnaFolder::getChild(int index, QObject *parent)  {
    DlnaResource* child = Q_NULLPTR;

    if (index >= 0 && index < children.size()) {
        QFileInfo fileinfo = children.at(index);

        QMimeDatabase mimeDb;

        if (fileinfo.isDir()) {
            child = new DlnaFolder(fileinfo.absoluteFilePath(),
                                   parent != Q_NULLPTR ? parent : this);
        }
        else if (mimeDb.mimeTypeForFile(fileinfo).name().startsWith("audio/")) {
            child = new DlnaMusicTrackFile(fileinfo.absoluteFilePath(),
                                           parent != Q_NULLPTR ? parent : this);
        }
        else if (mimeDb.mimeTypeForFile(fileinfo).name().startsWith("video/")) {
            child = new DlnaVideoFile(fileinfo.absoluteFilePath(),
                                      parent != Q_NULLPTR ? parent : this);
        }
        else {
            qCritical() << QString("Unkwown format %1: %2").arg(mimeDb.mimeTypeForFile(fileinfo).name(), fileinfo.absoluteFilePath());
        }
    }

    if (child)
    {
        child->setId(QString("%1").arg(index+1));
        child->setDlnaParent(this);
    }

    return child;
}

