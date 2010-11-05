
ALTER TABLE Catalog ADD COLUMN sortKey INT default 0;
ALTER TABLE Catalog ADD COLUMN lastUsed DATETIME;
ALTER TABLE Catalog ADD COLUMN useCounter INT default 0;

