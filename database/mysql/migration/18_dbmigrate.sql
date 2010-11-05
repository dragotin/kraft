# message Adding sort column and usage statistics
ALTER TABLE Catalog ADD COLUMN sortKey INT(11) default 0;
ALTER TABLE Catalog ADD COLUMN lastUsed DATETIME;
ALTER TABLE Catalog ADD COLUMN useCounter INT(11) default 0;

