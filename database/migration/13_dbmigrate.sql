# message Adding a taxType column
ALTER TABLE docposition ADD COLUMN taxType int default 0 AFTER price;
