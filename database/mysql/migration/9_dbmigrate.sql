# message add a document type id to text table
alter table DocTexts add column docTypeId int after docType; 

# message populate the doc type id column in docTexts
update DocTexts set docTypeId=( SELECT docTypeID FROM DocTypes WHERE name=docType ); 

# message create a type column for the docposition 
alter table docposition add column postype VARCHAR(64) AFTER text;

# message create type column for the archdocpos table
alter table archdocpos add column postype VARCHAR(64) AFTER kind;
