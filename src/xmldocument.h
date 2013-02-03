#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include "kode/document.h"

class KraftDoc;

using namespace KraftXml;

class XmlDocument : public Kraftdocument
{
public:
    XmlDocument();
    // write the KraftDoc from the pointer
    void setKraftDoc( KraftDoc* );

    // read the KraftDoc into the pointer
    void getKraftDoc( KraftDoc* );

};

#endif // XMLDOCUMENT_H
