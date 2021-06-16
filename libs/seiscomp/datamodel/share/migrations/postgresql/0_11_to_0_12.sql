\echo Add Origin.m_uncertainty_confidenceLevel
ALTER TABLE Origin ADD m_uncertainty_confidenceLevel DOUBLE PRECISION;

\echo Updating Meta
UPDATE Meta SET value='0.12' WHERE name='Schema-Version';
