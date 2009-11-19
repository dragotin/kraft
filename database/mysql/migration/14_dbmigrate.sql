# message Add tax table

CREATE TABLE taxes (
    id          INT NOT NULL AUTO_INCREMENT,
    fullTax     DECIMAL(5,1),
    reducedTax  DECIMAL(5,1),
    startDate   DATE,

    PRIMARY KEY( id )
);
INSERT INTO taxes ( fullTax, reducedTax, startDate ) VALUES (16.0, 7.0, '1998-04-01' );
INSERT INTO taxes ( fullTax, reducedTax, startDate ) VALUES (19.0, 7.0, '2007-01-01' );
