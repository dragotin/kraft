ALTER TABLE stockMaterial ADD COLUMN sortKey INT default 0;

CREATE TABLE IF NOT EXISTS catItemUsage (
  catId  INT NOT NULL,
  itemId INT NOT NULL,
  usageCount INT default 0,
  lastUsed DATETIME,

  PRIMARY KEY(catId, itemId)
);

ALTER TABLE Catalog DROP COLUMN lastUsed;
ALTER TABLE Catalog DROP COLUMN useCounter;

