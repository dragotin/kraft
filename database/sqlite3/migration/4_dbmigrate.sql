-- message Create DocTexts table

CREATE TABLE DocTexts (
  docTextID      INTEGER PRIMARY KEY ASC autoincrement,

  name           VARCHAR(64),  
  description    TEXT,
  text           TEXT,
  docType        VARCHAR( 64 ),
  docTypeId      int,
  textType       VARCHAR( 64 ),
  modDate        TIMESTAMP(14)
  
--   INDEX( docType, textType )
);
CREATE INDEX DocTextsIndx_4 ON DocTexts (docType, textType);
CREATE TRIGGER update_docTexts AFTER UPDATE ON DocTexts
BEGIN
  UPDATE DocTexts SET modDate = DATETIME('NOW')  WHERE docTextID = new.docTextID;
END;

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Offer', 'Header Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Offer', 'Footer Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Invoice', 'Header Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Invoice', 'Footer Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Acceptance of Order', 'Header Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Acceptance of Order', 'Footer Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Angebot', 'Kopf Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Angebot', 'Fuß Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Rechnung', 'Kopf Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Rechnung', 'Fuß Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Auftragsbestätigung', 'Kopf Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Auftragsbestätigung', 'Fuß Text' );


INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Offer', 'Footer Text' FROM wordLists WHERE category='docFooter_Offer';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Offer', 'Header Text' FROM wordLists WHERE category='docHeader_Offer';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Invoice', 'Footer Text' FROM wordLists WHERE category='docFooter_Invoice';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Invoice', 'Header Text' FROM wordLists WHERE category='docHeader_Invoice';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Acceptance of Order', 'Footer Text' FROM wordLists WHERE category='docFooter_Acceptance of Order';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Acceptance of Order', 'Header Text' FROM wordLists WHERE category='docHeader_Acceptance of Order';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Angebot', 'Fuß Text' FROM wordLists WHERE category='docFooter_Angebot';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Angebot', 'Kopf Text' FROM wordLists WHERE category='docHeader_Angebot';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Rechnung', 'Fuß Text' FROM wordLists WHERE category='docFooter_Rechnung';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Rechnung', 'Kopf Text' FROM wordLists WHERE category='docHeader_Rechnung';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Auftragsbestätigung', 'Fuß Text' FROM wordLists WHERE category='docFooter_Auftragsbestätigung';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Auftragsbestätigung', 'Kopf Text' FROM wordLists WHERE category='docHeader_Auftragsbestätigung';

