# message Creating attributes table...
CREATE TABLE attributes (
  hostObject VARCHAR(255),
  hostId     INT,
  name       VARCHAR(255),
  value      MEDIUMTEXT,

  PRIMARY KEY( hostObject, hostId)
);

# message Creating attributes for archived documents
CREATE TABLE archdocAttribs (
  archDocID  INT NOT NULL AUTO_INCREMENT,
  name       VARCHAR(255),
  value      VARCHAR(255),

  PRIMARY KEY( archDocID )
);

# message Adding position type and overall price ot archdocpositions
ALTER TABLE archdocpos ADD COLUMN kind VARCHAR(64) AFTER ordNumber;
ALTER TABLE archdocpos ADD COLUMN overallPrice DECIMAL(10,2) AFTER price;

# message Changing old kinds to Normal
UPDATE archdocpos SET kind = "Normal";

# message Calculating archive position price
UPDATE archdocpos SET overallPrice = ROUND( price * amount, 2);

