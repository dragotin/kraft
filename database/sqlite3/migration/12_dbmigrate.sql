CREATE TABLE numberCycles (
  id INTEGER PRIMARY KEY ASC autoincrement,      
  name VARCHAR(64) NOT NULL,
  lastIdentNumber  INT NOT NULL,                          
  identTemplate VARCHAR(64) NOT NULL  
);                              
CREATE UNIQUE INDEX numCycleIdx_12 ON numberCycles( name );

INSERT INTO numberCycles (name, lastIdentNumber, identTemplate) VALUES 
  ("default", (SELECT ifnull( 1+MAX(docID), 1 ) FROM document), '%i-%yyyy' );
