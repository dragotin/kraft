# message Add tax table

CREATE TABLE taxes (
    id          INT NOT NULL AUTO_INCREMENT,
    fullTax     DECIMAL(5,2),
    reducedTax  DECIMAL(5,2),
    startDate   DATE,

    PRIMARY KEY( id )
);
