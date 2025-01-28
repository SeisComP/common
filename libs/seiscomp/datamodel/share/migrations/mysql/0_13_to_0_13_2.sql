SELECT 'Convert Comment.text to LONGTEXT' AS '';
ALTER TABLE Comment MODIFY text LONGTEXT;

SELECT 'Convert JournalEntry.parameters to LONGTEXT' AS '';
ALTER TABLE JournalEntry MODIFY parameters LONGTEXT;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.13.2' WHERE name='Schema-Version';
