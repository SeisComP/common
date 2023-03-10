SELECT 'Convert JournalEntry.parameters to BLOB' AS '';
ALTER TABLE JournalEntry MODIFY parameters blob;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.12.1' WHERE name='Schema-Version';
