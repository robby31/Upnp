#ifndef DIDLLITE_H
#define DIDLLITE_H

#include <QDomDocument>

class DidlLite : public QDomDocument
{

public:
    explicit DidlLite();

    QDomNode addElement(QDomNode node);

private:
    QDomElement m_didl;
};

#endif // DIDLLITE_H
