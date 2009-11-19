# message Adding a taxType column
ALTER TABLE docposition ADD COLUMN taxType int default 3 AFTER price;
