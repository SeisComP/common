\echo Convert JournalEntry.m_parameters to BYTEA
ALTER TABLE JournalEntry ALTER COLUMN m_parameters TYPE bytea USING m_parameters::bytea;

\echo Updating Meta
UPDATE Meta SET value='0.12.1' WHERE name='Schema-Version';
