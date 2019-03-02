--- Add a column predecessor to the document table

ALTER TABLE document ADD COLUMN predecessor VARCHAR(32);
ALTER TABLE archdoc  ADD COLUMN predecessor VARCHAR(32);

