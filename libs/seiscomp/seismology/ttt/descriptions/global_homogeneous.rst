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

Example configuration for P, S (seismic) and Is (infrasound):

.. code-block:: properties

   # The list of supported model names per interface
   ttt.homogeneous.tables = "5"

   # Geographic origin of the region. Expects 2 values: latitude, longitude.
   ttt.homogeneous.5.origin = 51, 12

   # Maximum radius around origin
   ttt.homogeneous.5.radius = 1

   # Minimum source depth
   ttt.homogeneous.5.minDepth = 0

   # Maximum source depth
   ttt.homogeneous.5.maxDepth = 2

   # Phases and velocities
   ttt.homogeneous.5.velocities = P:5,S:3,Is:0.3


Application
===========

Once the travel-time interface profile is defined and registered, in can be
selected

* Interactively in the :ref:`scolv phase picker <scolv-sec-waveform-review>`
  or the :ref:`scolv amplitude picker <scolv-sec-amplitude-review>`,
* Or used in other modules which allow the configuration of travel-time
  interfaces or locators such as :ref:`global_fixedhypocenter` or
  :ref:`global_stdloc`.
