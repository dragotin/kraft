DELETE FROM preisArten;
INSERT INTO preisArten VALUES (0, 'open');
INSERT INTO preisArten VALUES (1, 'manual');
INSERT INTO preisArten VALUES (2, 'calculated');

DELETE FROM CatalogSet;
INSERT INTO CatalogSet (name, description, catalogType, sortKey) VALUES 
( "Standard Templates", 
  "A set of templates suitable for business", "TemplCatalog", 1 );
SET @newCat := LAST_INSERT_ID();

DELETE FROM CatalogChapters;
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Work', 1, @newCat );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Machine', 2, @newCat  );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Concrete', 3, @newCat  );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Stones', 4, @newCat  );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Misc', 5, @newCat  );
INSERT INTO CatalogChapters (chapter, sortKey, catalogSetID) VALUES ('Transportation', 6, @newCat  );

INSERT INTO CatalogSet( name, description, catalogType, sortKey) VALUES ("Material", 
  "Material Catalog to Use in Calculations in Templates", "MaterialCatalog", 2 );
SET @newCat := LAST_INSERT_ID();

INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Sand etc.', 3, @newCat );
INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Stones', 2, @newCat );
INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Concreate', 1, @newCat );
INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Pipes', 4, @newCat );   
INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Wood', 5, @newCat );	      
INSERT INTO CatalogChapters ( chapter, sortKey, catalogSetID ) VALUES ('Art and Furnitures', 6, @newCat );

DELETE FROM units;
INSERT INTO units VALUES (0, 'm', 'Meter', 'm', 'Meter' );
INSERT INTO units VALUES (1, 'sm', 'Squaremeter', 'qm', 'Squaremeter' );
INSERT INTO units VALUES (2, 'cbm', 'Cubikmeter', 'cbm', 'Cubikmeter' );
INSERT INTO units VALUES (3, 'Bag.', 'Bag', 'Bag', 'Bags' );
INSERT INTO units VALUES (4, 'l', 'Liter', 'l', 'Liter' );
INSERT INTO units VALUES (5, 'kg', 'Kilogramm', 'kg', 'Kilogramm' );
INSERT INTO units VALUES (6, 'Pcs.', 'Piece', 'Pcs.', 'Pieces' );
INSERT INTO units VALUES (7, 't', 'Ton', 't', 'Tons' );
INSERT INTO units VALUES (8, 'pausch.', 'pauschal', 'pausch.', 'pauschal' );
INSERT INTO units VALUES (9, 'Hour', 'Hour', 'Hours', 'Hours' );


DELETE FROM stdSaetze;
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Worker', 34.00, 1 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Master', 39.00, 2 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Helper', 30.00, 4 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Trainee', 21.00, 3 );
INSERT INTO stdSaetze (name, price, sortKey) VALUES ('Machine Driver', 33.00, 5 );

DELETE FROM wordLists;
INSERT INTO wordLists VALUES ('greeting', 'with kind regards,' );
INSERT INTO wordLists VALUES ('greeting', 'with best regards,' );
INSERT INTO wordLists VALUES ('greeting', 'yours faithfully,' );
INSERT INTO wordLists VALUES ('greeting', 'goodbye and thanks for the fish,' );
INSERT INTO wordLists VALUES ('greeting', 'goodbye' );
INSERT INTO wordLists VALUES ('greeting', 'forever yours,' );

INSERT INTO wordLists VALUES ('salut', 'Dear Mr. %NAME' );
INSERT INTO wordLists VALUES ('salut', 'Dear Mrs. %NAME' );
INSERT INTO wordLists VALUES ('salut', 'Dear Mrs. %NAME, dear Mr. %NAME' );
INSERT INTO wordLists VALUES ('salut', 'Dear %GIVEN_NAME' );
