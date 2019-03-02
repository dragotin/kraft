ALTER TABLE document ADD COLUMN predecessor VARCHAR(32) AFTER projectLabel;
ALTER TABLE archdoc ADD COLUMN predecessor VARCHAR(32) AFTER projectLabel;

UPDATE document SET predecessor = 0;
UPDATE archdoc  SET predecessor = 0;

