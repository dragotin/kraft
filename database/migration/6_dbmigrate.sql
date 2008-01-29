# message Localisation information on document level
ALTER TABLE document ADD country VARCHAR(32) AFTER posttext;
ALTER TABLE document ADD language VARCHAR(32) AFTER country;

ALTER TABLE archdoc ADD country VARCHAR(32) AFTER posttext;
ALTER TABLE archdoc ADD language VARCHAR(32) AFTER country;

