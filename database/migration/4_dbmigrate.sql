CREATE TABLE DocTexts (
  docTextID      INT NOT NULL AUTO_INCREMENT,

  name           VARCHAR(64),  
  description    TEXT,
  text           TEXT,
  docType        VARCHAR( 64 ),
  textType       VARCHAR( 64 ),
  modDate        TIMESTAMP(14),

  PRIMARY KEY( docTextID ),
  INDEX( docType, textType )
);
