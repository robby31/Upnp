#ifndef DLNAROOTFOLDER_H
#define DLNAROOTFOLDER_H

#include <QFileInfo>
#include "dlnastoragefolder.h"
#include "dlnafolder.h"

class DlnaRootFolder : public DlnaStorageFolder
{
    Q_OBJECT

public:
    explicit DlnaRootFolder(QObject *parent = Q_NULLPTR);
    ~DlnaRootFolder() Q_DECL_OVERRIDE;

    // Any resource needs to represent the container or item with a String.
    // String to be showed in the UPNP client.
    QString getName() const Q_DECL_OVERRIDE { return QString("root"); }

    QString getSystemName() const Q_DECL_OVERRIDE { return getName(); }

    // Returns the DisplayName that is shown to the Renderer.
    QString getDisplayName() const Q_DECL_OVERRIDE { return getName(); }

    int getChildrenSize() const Q_DECL_OVERRIDE { return children.size(); }
    DlnaResource* getChild(int index, QObject *parent = Q_NULLPTR) Q_DECL_OVERRIDE;

    /*
     * Adds a new DLNAResource to the child list.
     * Only useful if this object is of the container type.
     */
    void addChild(DlnaResource *child)  { emit addChildSignal(child); }
    void clearChildren()                { emit clearChildrenSignal(); }

    void addFolder(const QString& path) { emit addFolderSignal(path); }

signals:
    void addFolderSignal(const QString &path);
    void folderAddedSignal(const QString &folder);
    void error_addFolder(QString folder);

    void addChildSignal(DlnaResource *child);
    void clearChildrenSignal();

private slots:
    // returns true if the folder is added to Root.
    virtual bool addFolderSlot(QString folder);

    void addChildSlot(DlnaResource *child);
    void clearChildrenSlot() { children.clear(); }

private:
    QList<DlnaResource*> children;

public:
    static qint64 objectCounter;
};

#endif // DLNAROOTFOLDER_H
