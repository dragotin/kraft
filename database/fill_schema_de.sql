
INSERT INTO preisArten VALUES (0, 'offen');
INSERT INTO preisArten VALUES (1, 'selbsterstellt');
INSERT INTO preisArten VALUES (2, 'kalkuliert');

INSERT INTO CatalogSet (name, description, sortKey) VALUES ( "Mustertexte GALA-Bau", \
  "Kalkulierte Musterposten für den Garten- und Landschaftsbau", 1 );
SET @newCat := LAST_INSERT_ID();

INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Arbeit', 1 );
INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Maschine', 2 );
INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Materialeinsatz', 3 );
INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Service', 4 );
INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Sonstige', 5 );
INSERT INTO CatalogChapters (chapter, sortKey) VALUES ('Transport', 6 );
UPDATE CatalogChapters SET catalogSetID=@newCat;

INSERT INTO units VALUES (0, 'm', 'Meter', 'm', 'Meter' );
INSERT INTO units VALUES (1, 'qm', 'Quadratmeter', 'qm', 'Quadratmeter' );
INSERT INTO units VALUES (2, 'cbm', 'Kubikmeter', 'cbm', 'Kubikmeter' );
INSERT INTO units VALUES (3, 'Sck.', 'Sack', 'Sck.', 'Saecke' );
INSERT INTO units VALUES (4, 'l', 'Liter', 'l', 'Liter' );
INSERT INTO units VALUES (5, 'kg', 'Kilogramm', 'kg', 'Kilogramm' );
INSERT INTO units VALUES (6, 'Stck.', 'Stueck', 'Stck.', 'Stueck' );
INSERT INTO units VALUES (7, 't', 'Tonne', 't', 'Tonnen' );
INSERT INTO units VALUES (8, 'pausch.', 'pauschal', 'pausch.', 'pauschal' );

INSERT INTO matKats VALUES(1, "Material Allgemein");
UPDATE stockMatChapter SET matKatID=1;

INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Geselle', 34.00, 1 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Meister', 39.00, 2 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Helfer', 30.00, 4 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Auszubildender', 21.00, 3 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Maschinenfuehrer', 33.00, 5 );
