CREATE TABLE DocTexts (
  docTextID      INT NOT NULL AUTO_INCREMENT,
  
  text           TEXT,
  description    TEXT,
  docType        VARCHAR( 64 ),
  textType       VARCHAR( 64 ),
  modDate        TIMESTAMP(14),

  PRIMARY KEY( docTextID ),
  INDEX( docType, textType )
);
