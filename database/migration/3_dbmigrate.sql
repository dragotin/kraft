CREATE TABLE plantPrices (
  matchCode         VARCHAR(255),
  price             DECIMAL(8,2),
  lastUpdate        TIMESTAMP,

  PRIMARY KEY( matchCode )
);

ALTER TABLE document ADD COLUMN docDescription TEXT AFTER docType;

