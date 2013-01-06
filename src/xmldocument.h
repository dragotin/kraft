#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include "kode/document.h"

class KraftDoc;

using namespace KraftXml;

class XmlDocument : public Kraftdocument
{
public:
    XmlDocument();
    void setKraftDoc( KraftDoc *doc );

};

#endif // XMLDOCUMENT_H
