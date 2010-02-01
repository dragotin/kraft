-- message Add plant Prices table
CREATE TABLE plantPrices (
  matchCode         VARCHAR(255),
  price             DECIMAL(8,2),
  lastUpdate        TIMESTAMP,

  PRIMARY KEY( matchCode )
);

CREATE TRIGGER update_plantPrices AFTER UPDATE ON plantPrices
BEGIN
  UPDATE plantPrices SET lastUpdate = DATETIME('NOW')  WHERE matchCode = new.matchCode;
END;

-- Columns already added in create_schema.sql   *sqlite workaround*
--ALTER TABLE document ADD COLUMN docDescription TEXT AFTER docType;

