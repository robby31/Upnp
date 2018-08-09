#ifndef DIDLLITE_H
#define DIDLLITE_H

#include <QDomDocument>

class DidlLite : public QDomDocument
{

public:
    explicit DidlLite();

    QDomNode addElement(const QDomNode& node);

private:
    QDomElement m_didl;
};

#endif // DIDLLITE_H
