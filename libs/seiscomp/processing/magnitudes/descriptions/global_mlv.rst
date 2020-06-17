Amplitude
---------

The MLv amplitude calculation is very similar to the original :ref:`ML<global_ml>`,
except that the amplitude is measured on the vertical component.

Station Magnitude
-----------------

The individual station MLv is calculated up to the epicentral distance maxDistanceKm
using the following formula:

.. math::

   MLv = \log10(A) - \log10(A0)

A is the MLv Wood-Anderson amplitude in millimeters. The second term
is the empirical calibration function, which in turn is a function
of the epicentral distance (see Richter, 1935). This calibration
function can be configured globally or per station using the config
variable module.trunk.global.MLv.logA0, e.g. ::

   module.trunk.global.MLv.logA0 = "0 -1.3;60 -2.8;100 -3.0;400 -4.5;1000 -5.85"

The logA0 configuration string consists of an arbitrary number of
distance-value pairs separated by semicolons. The distance is in km
and the value corresponds to the log10(A0) term above.

Within each interval the values are computed by linear
interpolation. E.g. for the above default specification, at a
distance of 80 km the log10(A0) value would be

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
* Time window: 150 s by :ref:`scautopick` or distance dependent
* Default distance range: 0 - 8 deg
* Depth range: no limitation

Network magnitude
-----------------

By default, the trimmed mean is calculated from the station magnitudes to form
the :term:`network magnitude`. Outliers below the 12.5% and above the 12.5% percentiles are
removed before the calculation.

Configuration
-------------

Several distance-value pairs can be configured for different ranges of
epicentral distance.
The calibration function and maximum distance can be configured globally,
per network or per station using the configuration variables, e.g.

global:

.. code-block:: sh

   module.trunk.global.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.global.MLv.maxDistanceKm = -1

or per network:

.. code-block:: sh

   module.trunk.GR.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.GR.MLv.maxDistanceKm = -1

or per station:

.. code-block:: sh

   module.trunk.GR.MOX.MLv.logA0 = "0 -1.3;60 -2.8;400 -4.5;1000 -5.85"
   module.trunk.GR.MOX.MLv.maxDistanceKm = -1

Set the configuration and calibration parameters in the global bindings. By deault MLv is computed
by :ref:`scautopick` and is visible in the GUI.
