ML is the standard local (Richter) magnitude originally designed for
Southern California by :cite:t:`richter-1935`.

General (default) conditions apply:

* Amplitude unit in SeisComP: **millimeter** (mm) by simulation of a :term:`Wood-Anderson seismometer`.
* Time window, configurable: 150 s by :ref:`scautopick` or distance dependent, configurable.
* Distance type: epicentral distance.
* Distance range: 0 - 8 deg,  maximum is configurable:
  :confval:`magnitudes.ML.maxDistanceKm`,
  measurements beyond 8 deg will be strictly ignored.
* Depth range: 0 - 80 km, configurable for amplitude measurements.


Amplitudes
----------

The ML amplitude calculation is similar to the original ML. Waveforms from both
horizontal components are time-windowed and restituted to the Wood-Anderson
seismograph. Within the time window the amplitudes are measured on both
horizontal components and combined. The methods for measuring and combining
amplitudes are configurable in the global bindings.


Station Magnitudes
------------------

The individual station ML is calculated using the following formula:

.. math::

   ML = \log10(A) - \log10(A0)

*A* is the measured ML Wood-Anderson amplitude in millimeters. The second term
is the empirical calibration function, which in turn is a function
of the epicentral distance (:cite:t:`richter-1935`). This calibration
function and distance range can be configured globally or per station using
global bindings or the global module configuration variable
module.trunk.global.magnitudes.ML.logA0 in :file:`global.cfg`, e.g.

.. code-block:: params

   module.trunk.global.magnitudes.ML.logA0 = "0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85"
   module.trunk.global.magnitudes.ML.maxDistanceKm = "-1"

The *logA0* configuration string consists of an arbitrary number of
distance-value pairs separated by comma. Within the pairs, the values are
separated by colon. The distance is epicentral distance in km
and the second value corresponds to the *log10(A0)* term above.

Within each interval the values are computed by linear
interpolation. E.g. for the above default specification, at a
distance of 80 km the *log10(A0)* value would be

.. math::

   \log10(A0) &= ((-3.0)-(-2.8))*(80-60)/(100-60)-2.8 \\
              &= -2.9

In other words, at 80 km distance the magnitude would be

.. math::

   ML &= \log10(A) - (-2.9) \\
      &= \log10(A) + 2.9

which is according to the original Richter formula :cite:p:`richter-1935` if the
amplitude is measured in millimeters.

Several distance-value pairs can be configured for different ranges of
epicenter distance.


Network magnitude
-----------------

By default, the mean is calculated from the station magnitudes to form the
network magnitude.


Configuration
-------------

Set the configuration and calibration parameters in the global bindings similar
to :ref:`global_mlv`.
Instead configuring lots of global bindings profiles or station bindings one
line per parameter can be added to the global module configuration
(:file:`global.cfg`).

Add ML to the list of computed amplitudes and magnitudes in the configuration of
:ref:`scamp` and :ref:`scmag` and in :ref:`scesv` or :ref:`scolv` for visibility.
