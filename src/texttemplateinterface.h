#ifndef TextTemplateInterface_H
#define TextTemplateInterface_H

#include <QString>

class TextTemplateInterface
{
public:
    TextTemplateInterface();

    virtual ~TextTemplateInterface();

    /**
     * take the template absolute filename of the template source and
     * load it immediately.
     * returns true if successful. Otherwise check errorString() for
     * error messages
     */
    bool setTemplateFileName(const QString& file);
    QString fileName() const;

    /**
     * @brief isOk - returns true if the TextTemplate is ok and can be used.
     * @return true if the text template can be used.
     */
    virtual bool isOk();

    /**
     * return a describing string if something went wrong when opening
     * the template.
     */
    QString errorString() const { return _errorString; }

    /**
     * set a value in the default dictionary
     */
    virtual void setValue( const QString&, const QString& ) = 0;

    /**
     * get the expanded output
     */
    virtual QString expand() = 0;

protected:
    /**
     * @brief initialize - use for basic initialization
     * @return true if successful
     *
     * This method can assume that the filename member var points to
     * a valid file.
     */
    virtual bool initialize() = 0;

    /**
     * @brief overwrites the error message
     * @param the error string
     */
    void setError(const QString& msg) { _errorString = msg; }

private:
    QString _fileName;
    QString _errorString;
};

#endif // TextTemplateInterface_H
