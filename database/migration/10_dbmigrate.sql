# message Add a list value identification column to the attribute table

ALTER TABLE attributes ADD COLUMN valueIsList tinyint default 0 after value;

ALTER TABLE attributes DROP PRIMARY KEY;
ALTER TABLE attributes ADD COLUMN id int not null  auto_increment primary key FIRST;
ALTER TABLE attributes ADD UNIQUE INDEX ( hostObject, hostId, name );

# message Create an attribute value table
CREATE TABLE attributeValues (
  id INT NOT NULL AUTO_INCREMENT,
  attributeId INT NOT NULL,
  value      VARCHAR(255),

  PRIMARY KEY( id ),
  INDEX( attributeId )
);

# message create a table to keep tag templates
CREATE TABLE `tagTemplates` (
  `tagTmplID` int(11) NOT NULL auto_increment,
  `sortkey` int(11) NOT NULL,
  `name` varchar(255) default NULL,
  `description` varchar(255) default NULL,
  `color` char(7) default NULL,
  PRIMARY KEY  (`tagTmplID`),
  KEY `sortkey` (`sortkey`)
);

