# message Creating attributes table...
CREATE TABLE attributes (
  hostObject VARCHAR(255),
  hostId     INT,
  name       VARCHAR(255),
  value      MEDIUMTEXT,

  PRIMARY KEY( hostObject, hostId)
);

ALTER TABLE archdocpos ADD COLUMN kind VARCHAR(64) AFTER ordNumber;
ALTER TABLE archdocpos ADD COLUMN overallPrice DECIMAL(10,2) AFTER price;

UPDATE archdocpos SET kind = "Normal";
UPDATE archdocpos SET overallPrice = ROUND( price * amount, 2);
