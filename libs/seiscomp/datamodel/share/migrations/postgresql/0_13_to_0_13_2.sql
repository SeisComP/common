\echo Convert Comment.m_text to TEXT
ALTER TABLE Comment ALTER COLUMN m_text TYPE TEXT;

\echo Convert JournalEntry.m_parameters to TEXT
ALTER TABLE JournalEntry ALTER COLUMN m_parameters TYPE TEXT;

\echo Updating Meta
UPDATE Meta SET value='0.13.2' WHERE name='Schema-Version';
