DELETE FROM preisArten;
INSERT INTO preisArten (preisArt) VALUES ('offen');
INSERT INTO preisArten (preisArt) VALUES ('selbsterstellt');
INSERT INTO preisArten (preisArt) VALUES ('kalkuliert');

DELETE FROM CatalogSet;
INSERT INTO CatalogSet (name, description, catalogType, sortKey) VALUES 
 ( "Standard Mustertexte", "Kalkulierte Musterposten", "TemplCatalog", 1 );

DELETE FROM CatalogChapters;
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Arbeit', 1, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte"));
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Maschine', 2, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte") );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES
  ('Materialeinsatz', 3, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte") );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Service', 4, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte") );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Sonstige', 5, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte") );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES
  ('Transport', 6, (SELECT catalogSetID FROM CatalogSet WHERE name="Standard Mustertexte") );

INSERT INTO CatalogSet( name, description, catalogType, sortKey) VALUES ("Material",
  "Materialkatalog", "MaterialCatalog", 2 );

INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Schüttgüter', 3, (SELECT catalogSetID FROM CatalogSet WHERE name="Material"));
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Naturstein', 2, (SELECT catalogSetID FROM CatalogSet WHERE name="Material"));
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Beton', 1, (SELECT catalogSetID FROM CatalogSet WHERE name="Material"));
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES 
  ('Rohre', 4, (SELECT catalogSetID FROM CatalogSet WHERE name="Material"));

DELETE FROM units;
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('m', 'Meter', 'm', 'Meter' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('qm', 'Quadratmeter', 'qm', 'Quadratmeter' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('cbm', 'Kubikmeter', 'cbm', 'Kubikmeter' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('Sck.', 'Sack', 'Sck.', 'Säcke' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ( 'l', 'Liter', 'l', 'Liter' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('kg', 'Kilogramm', 'kg', 'Kilogramm' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('Stck.', 'Stueck', 'Stck.', 'Stück' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('t', 'Tonne', 't', 'Tonnen' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('pausch.', 'pauschal', 'pausch.', 'pauschal' );
INSERT INTO units (unitShort, unitLong, unitPluShort, unitPluLong) VALUES 
                  ('Std.', 'Stunde', 'Std.', 'Stunden' );

DELETE FROM stdSaetze;
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Geselle', 34.00, 1 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Meister', 39.00, 2 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Helfer', 30.00, 4 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Auszubildender', 21.00, 3 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Maschinenführer', 33.00, 5 );

DELETE FROM wordLists;
INSERT INTO wordLists VALUES ('greeting', 'mit den besten Grüssen,' );
INSERT INTO wordLists VALUES ('greeting', 'liebe Grüsse,' );
INSERT INTO wordLists VALUES ('greeting', 'Hochachtungsvoll,' );
INSERT INTO wordLists VALUES ('greeting', 'mit freundlichem Gruß,' );

INSERT INTO wordLists VALUES ('salut', 'Sehr geehrter Herr %NAME,' );
INSERT INTO wordLists VALUES ('salut', 'Sehr geehrte Frau %NAME,' );
INSERT INTO wordLists VALUES ('salut', 'Sehr geehrte Frau %NAME, sehr geehrter Herr %NAME,' );
INSERT INTO wordLists VALUES ('salut', 'Lieber %GIVEN_NAME,' );
INSERT INTO wordLists VALUES ('salut', 'Liebe %GIVEN_NAME,' );


