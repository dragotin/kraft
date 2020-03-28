#include "texttemplateinterface.h"
#include "defaultprovider.h"

#include <QFileInfo>
#include <QDebug>

#include <klocalizedstring.h>

namespace {

QString findTemplateFile(const QString &filename)
{
    QString tmplFile;
    if( ! filename.isEmpty() ) {
        const QString templFileName = QStringLiteral("kraft/")+filename;

        tmplFile = DefaultProvider::self()->locateFile(templFileName);
    }
    return tmplFile;
}

} // end of anonym namespace

TextTemplateInterface::TextTemplateInterface()
{

}

TextTemplateInterface::~TextTemplateInterface()
{

}

QString TextTemplateInterface::fileName() const
{
    return _fileName;
}

bool TextTemplateInterface::setTemplateFileName( const QString& name )
{
  _errorString.clear();

  _fileName = name;

  QFileInfo info( _fileName );

  if ( info.isAbsolute() ) {
    // assume it is a absolute path
  } else {
    _fileName = findTemplateFile(_fileName);

    if ( _fileName.isEmpty() ) {
      _errorString = i18n( "No file name given for template" );
      return false;
    }

    info.setFile( _fileName );
  }

  if ( ! ( info.isFile() && info.isReadable() ) ) {
    _errorString = i18n( "Could not find template file %1", info.absoluteFilePath() );
    return false;
  }

  qDebug () << "Loading template source file: " << _fileName << endl;
  return initialize();
}

bool TextTemplateInterface::isOk()
{
    return _errorString.isEmpty();
}
