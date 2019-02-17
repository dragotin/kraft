
#ifndef T_SQLDEFS
#define T_SQLDEFS

#define CREATE_ATTRIBUTES "CREATE TABLE attributes ( \
  hostObject            VARCHAR(64), \
  hostId                INT NOT NULL, \
  name                  VARCHAR(64),  \
  value                 MEDIUMTEXT,   \
  valueIsList           tinyint default 0, \
  relationTable         VARCHAR(64) default NULL, \
  relationIDColumn      VARCHAR(64) default NULL, \
  relationStringColumn  VARCHAR(64) default NULL, \
  PRIMARY KEY( hostObject, hostId, name ) \
);"

#define CREATE_DOCTYPES "CREATE TABLE DocTypes ( \
  docTypeID INTEGER PRIMARY KEY ASC autoincrement, \
  name      VARCHAR(255) \
  );"



#endif

