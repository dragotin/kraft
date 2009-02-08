CREATE TABLE numberCycles (                       
  id INT NOT NULL AUTO_INCREMENT,      
  name VARCHAR(64) NOT NULL,
  lastIdentNumber  INT NOT NULL,                          
  identTemplate VARCHAR(64) NOT NULL,

  PRIMARY KEY( id ),
  UNIQUE(name)
);                              

SELECT @id := IF( ISNULL(MAX( docID)), 1, MAX(docID) ) FROM document;

INSERT INTO numberCycles (name, lastIdentNumber) VALUES ("default", @id);
