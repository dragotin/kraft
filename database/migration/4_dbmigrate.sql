CREATE TABLE DocTexts (
  docTextID      INT NOT NULL AUTO_INCREMENT,

  name           VARCHAR(64),  
  description    TEXT,
  text           TEXT,
  docType        VARCHAR( 64 ),
  textType       VARCHAR( 64 ),
  modDate        TIMESTAMP(14),

  PRIMARY KEY( docTextID ),
  INDEX( docType, textType )
);

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
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Angebot', 'Fuﬂ Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Rechnung', 'Kopf Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Rechnung', 'Fuﬂ Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Auftragsbest√§tigung', 'Kopf Text' );

INSERT INTO DocTexts ( name, text, docType, textType ) 
VALUES ( 'Standard', 'Please edit me - Bitte passe mich an!', 'Auftragsbest√§tigung', 'Fuﬂ Text' );


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
SELECT 'Standard', word, 'Angebot', 'Fuﬂ Text' FROM wordLists WHERE category='docFooter_Angebot';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Angebot', 'Kopf Text' FROM wordLists WHERE category='docHeader_Angebot';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Rechnung', 'Fuﬂ Text' FROM wordLists WHERE category='docFooter_Rechnung';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Rechnung', 'Kopf Text' FROM wordLists WHERE category='docHeader_Rechnung';

INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Auftragsbest√§tigung', 'Fuﬂ Text' FROM wordLists WHERE category='docFooter_Auftragsbest√§tigung';
INSERT INTO DocTexts ( name, text, docType, textType )
SELECT 'Standard', word, 'Auftragsbest√§tigung', 'Kopf Text' FROM wordLists WHERE category='docHeader_Auftragsbest√§tigung';

