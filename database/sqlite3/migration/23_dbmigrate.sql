ALTER TABLE stockMaterial ADD COLUMN sortKey INT default 0;

CREATE TABLE catItemUsage (
  catId  INT NOT NULL,
  itemId INT NOT NULL,
  usageCount INT default 0,
  lastUsed DATETIME,

  PRIMARY KEY(catId, itemId)
);

-- These statements are only supported starting from 3.35
-- https://sqlite.org/changes.html#version_3_35_0
-- rather disabled for now for robustness
-- ALTER TABLE Catalog DROP COLUMN lastUsed;
-- ALTER TABLE Catalog DROP COLUMN useCounter;
