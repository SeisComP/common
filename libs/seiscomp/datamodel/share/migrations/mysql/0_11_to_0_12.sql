SELECT 'Add Origin.confidenceLevel' AS '';
ALTER TABLE Origin ADD uncertainty_confidenceLevel DOUBLE UNSIGNED AFTER uncertainty_preferredDescription;

SELECT 'Updating Meta' AS '';
UPDATE Meta SET value='0.12' WHERE name='Schema-Version';
