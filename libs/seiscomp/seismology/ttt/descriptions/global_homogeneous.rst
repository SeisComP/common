The travel-time interface *homogeneous* allows predicting travel times for
P and S wave and homogeneous velocity models.


Configuration
=============

The travel-time interface *homogeneous* is controlled by global parameters,
e.g., in :file:`$SEISCOMP_ROOT/etc/global.cfg`:

#. Add a new table profile for homogeneous travel-time tables with some custom
   profile name. In :ref:`scconfig` navigate to the section *ttt.homogeneous*
   and click on the green button to add a table profile.
#. Set all parameters in the new profile.
#. Register the new profile by adding its name to the list of tables in
   :confval:`ttt.homogeneous.tables`

Example configuration:

.. code-block:: params

   # The list of supported model names per interface.
   ttt.homogeneous.tables = "5"

   # Geographic origin of the region. Expects 2 values: latitude, longitude.
   ttt.homogeneous.5.origin = 51, 12

   # Radius validity of the region.
   ttt.homogeneous.5.radius = 1

   # Min Depth validity of the region.
   ttt.homogeneous.5.minDepth = 0

   # Max Depth validity of the region.
   ttt.homogeneous.5.maxDepth = 2

   # P wave velocity.
   ttt.homogeneous.5.P-velocity = 5

   # S wave velocity.
   ttt.homogeneous.5.S-velocity = 3


Application
===========

Once the travel-time interface profile is defined and registered, in can be
selected

* interactively in the :ref:`scolv phase picker <scolv-sec-waveform-review>`
  or the :ref:`scolv amplitude picker <scolv-sec-amplitude-review>`,
* or used in other modules which allow the configuration of travel-time
  interfaces.
