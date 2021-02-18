Amplitude
---------

The MLv amplitude calculation is very similar to the original :ref:`ML<global_ml>`,
except that the amplitude is measured on the vertical component. The methods
for measuring amplitudes are configurable in the global bindings.


Station Magnitude
-----------------

The individual station MLv is calculated up to the epicentral distance maxDistanceKm
using the following formula:

.. math::

   MLv = \log10(A) - \log10(A0)

A is the MLv Wood-Anderson amplitude in millimeters. The second term
is the empirical calibration function, which in turn is a function
of the epicentral distance (see Richter, 1935). This calibration
function can be configured globally or per station using global
bindings or the global module configuration variable
module.trunk.global.magnitudes.MLv.logA0 in :file:`global.cfg`, e.g. ::

   module.trunk.global.magnitudes.MLv.logA0 = "0 -1.3;60 -2.8;100 -3.0;400 -4.5;1000 -5.85"
   module.trunk.global.magnitudes.MLv.maxDistanceKm = "-1"

The logA0 configuration string consists of an arbitrary number of
distance-value pairs separated by semicolons. The distance is in km
and the value corresponds to the *log10(A0)* term above.

Within each interval the values are computed by linear
interpolation. E.g. for the above default specification, at a
distance of 80 km the *log10(A0)* value would be

.. math::

   \log10(A0) &= ((-3.0)-(-2.8))*(80-60)/(100-60)-2.8 \\
              &= -2.9

In other words, at 80 km distance the magnitude would be

.. math::

   MLv &= \log10(A) - (-2.9) \\
       &= \log10(A) + 2.9

which is according to the original Richter (1935) formula if the
amplitude is measured in millimeters.

* Amplitude unit in SeisComP: **millimeter** (mm)
* Time window: 150 s by :ref:`scautopick` or distance dependent, configurable
* Default distance range: 0 - 8 deg,  maximum is configurable :confval:`magnitudes.MLv.maxDistanceKm`,
  measurements beyond 8 deg will be strictly ignored.
* Depth range: no limitation


Network magnitude
-----------------

By default, the trimmed mean is calculated from the station magnitudes to form
the :term:`network magnitude`. Outliers beyond the outer 12.5% percentiles are
removed before forming the mean.


Configuration
-------------

Several distance-value pairs can be configured for different ranges of
epicentral distance.
The calibration function and maximum distance can be configured globally,
per network or per station using the configuration variables. Instead configuring
lots of global bindings profiles or station bindings one line per parameter can be
added to the global module configuration (:file:`global.cfg`), e.g.

global:

.. code-block:: sh

   module.trunk.global.magnitudes.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.global.magnitudes.MLv.maxDistanceKm = -1

or per network:

.. code-block:: sh

   module.trunk.GR.magnitudes.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.GR.magnitudes.MLv.maxDistanceKm = -1

or per station:

.. code-block:: sh

   module.trunk.GR.MOX.magnitudes.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.GR.MOX.magnitudes.MLv.maxDistanceKm = -1

Set the configuration and calibration parameters in the global bindings. By default MLv is computed
by :ref:`scautopick` and is visible in GUIs.
