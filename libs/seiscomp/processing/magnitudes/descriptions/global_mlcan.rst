MLcan is the custom magnitude for local events provided by the mlcan plugin.
The implementation is based on specifications by Spanish National Geographic Institute,
based on the corrections obtained by Mezcua and Rueda (2022): 
https://doi.org/10.1007/s00445-022-01553-9
This magnitude changes the c0 station correction for the c0_first
and c0_second, which corresponds to channel correction for a given station. 

The MLcan magnitude is very similar to the original :ref:`ML<global_ml>`,
except that by default

* Amplitude pre-filtering is applied.
* A parametric :ref:`magnitude calibration <mlc_station_magnitude>` function
  applies.
* Hypocentral distance is used.

Regionalization of magnitude computation is provided through global module
configuration.
Configuration of global bindings provides additional flexibility:

* Amplitudes can be pre-filtered before simulating a :term:`Wood-Anderson seismometer`
  (:confval:`amplitudes.MLcan.preFilter`),
* Wood-Anderson simulation is optional:
  :confval:`amplitudes.MLcan.applyWoodAnderson`,
* Measured amplitudes can be scaled accounting for expected unit
  (:confval:`amplitudes.MLcan.amplitudeScale`),
* A parametric or A0-based non-parametric
  :ref:`magnitude calibration <mlc_station_magnitude>`
  function can be applied as controlled by
  :confval:`magnitudes.MLcan.calibrationType`.
* Consider either hypocentral or epicentral distance for computing magnitudes
  (:confval:`magnitudes.MLcan.distMode`).

General (default) conditions apply, all values are configurable:

* Amplitude pre-filtering: :ref:`BW(3,0.5,12) <filter-bw>`.
* Amplitude unit in SeisComP: **millimeter** (mm) or as considered by the
  configured calibration parameters.
* Optional amplitude scaling: 1.
* :term:`Wood-Anderson seismometer` simulation: yes.
* Time window: 150 s by :ref:`scautopick` or distance dependent
  with :math:`endTime = min(R / 4, 150)`, configurable.
* Distance type: hypocentral.
* Distance range: 0 - 8 deg, measurements beyond 8 deg will be
  strictly ignored.
* Depth range: <= 80 km.
* Magnitude calibration: parametric.


Amplitudes
----------

MLcan amplitudes can be measured automatically by :ref:`scautopick` or :ref:`scamp`
or interactively by :ref:`scolv` very similarly to the original :ref:`ML<global_ml>`,
except that they can be pre-filtered and simulation of a :term:`Wood-Anderson seismometer`
is optional: :confval:`amplitudes.MLcan.preFilter`,
:confval:`amplitudes.MLcan.applyWoodAnderson`.
By default amplitudes are measured on both horizontal components where the
absolute maxima are taken. They are combined to a final measured amplitude by
taking the mean. The methods for measuring and combining the amplitudes are
configurable:
:confval:`amplitudes.MLcan.measureType`, :confval:`amplitudes.MLcan.combiner`.

The Wood-Anderson simulation will convert input velocity data to ground
displacement in mm. The input data may be of a different unit after applying
:confval:`amplitudes.MLcan.preFilter`, e.g. when integration is applied, and / or
when Wood-Anderson simulation is disabled. Configure
:confval:`amplitudes.MLcan.amplitudeScale` for converting the unit of the
processed data to the unit expected by the
:ref:`station magnitude calibration <mlc_station_magnitude>` for the measured
amplitude.

.. note::

   For comparing MLcan amplitudes with :ref:`ML amplitudes <global_ml>` set the
   global bindings parameters ::

      amplitudes.MLcan.preFilter = ""
      amplitudes.MLcan.combiner = average


.. _mlc_station_magnitude:

Station Magnitudes
------------------

Station magnitudes are computed from measured amplitudes automatically by
:ref:`scmag`
or interactively by :ref:`scolv`. By global bindings configuration MLcan considers

* Hypocentral (default) or epicentral distance: :confval:`magnitudes.MLcan.distMode`.
* Distance range: :confval:`magnitudes.MLcan.minDist`, :confval:`magnitudes.MLcan.maxDist`.
* Events with depth up to :confval:`magnitudes.MLcan.maxDepth`.
* Parametric or non-parametric calibration functions

  * parametric when :confval:`magnitudes.MLcan.calibrationType` = "parametric"`:

    .. math::

       MLcan = \log_{10}(A) + c_3 * \log_{10}(r/c_5) + c_2 * (r + c_4) + c_1 + c_0(station)

    where

    * *A*: displacement amplitude measured in unit of mm or as per configuration
    * *r*: hypocentral (default) or epicentral distance
    * *c1*, *c2*, *c3*, *c4*, *c5*: general calibration parameters
    * *c0*: channel-station-specific correction
    * *r*: Hypocentral (default) or epicentral distance as configured by
      :confval:`magnitudes.MLcan.distMode`.
    
    c0, is the correction for the component used in amplitude measurement. In case of average or 
    geometric average measurement, the mean correction for c0_first and c0_second will be used. 
    c0_first is the north component or the closest to the nort. c0 second is the east component
    or the closest to the east. 

  * A0-based non-parametric when :confval:`magnitudes.MLcan.calibrationType` = "A0"`:

    .. math::

       MLcan = \log_{10}(A) - \log_{10}(A_0)

    where

    * :math:`log_{10}(A_0)`: distance-dependent correction value. Read
      :ref:`global_mlv` for the details.

.. note::

   The magnitude calibration function can regionalized by adjusting global module
   configuration parameters in MLcan region profiles of
   :confval:`magnitudes.MLcan.region.*` and in a *MLcan* Magnitude type profile e.g.
   in :file:`global.cfg`.


Network Magnitude
-----------------

The network magnitude is computed from station magnitudes automatically by
:ref:`scmag` or interactively by :ref:`scolv`.
Originally the median was computed from all station MLcan to form the
:term:`network magnitude` MLcan. Here, the trimmed mean is applied. Outliers
beyond the outer 12.5% percentiles are removed before forming the mean. The
method can be adjusted in :ref:`scmag` by :confval:`magnitudes.average`.


Examples
--------

The flexibility of the amplitude and magnitude processing allows to apply MLcan
in various use cases, e.g.

* **Default:** Pre-filtered and gain-corrected amplitudes, Wood-Anderson
  corrected and measured in mm for Southwestern Germany, :cite:t:`stange-2006`:

  .. math::

     MLcan = \log_{10}(A) + 1.11 * \log_{10}(r) + 0.00095 * r + 0.69 + c_0

* Wood-Anderson-corrected displacement amplitudes measured in mm for
  Southern California, :cite:t:`hutton-1987`:

  .. math::

     MLcan = \log_{10}(A) + 1.110 * \log_{10}(r / 100) + 0.00189 * (r - 100) + 3.0

* Pre-filtered velocity amplitudes in units of mym/s (requiring to set
  :confval:`amplitudes.MLcan.amplitudeScale`), no Wood-Anderson correction,
  for West Bohemia, e.g. :cite:t:`hiemer-2012`:

  .. math::

     MLcan = \log_{10}(A) - log_{10}(2\Pi) + 2.1 * \log_{10}(r) - 1.7 + c_0

.. figure:: media/magnitude-calibrations_MLcan_s_MLcan_hb.png
   :align: center
   :width: 18cm

   MLcan magnitudes for measured amplitude of 1 mm with default magnitude
   calibration (*MLcan_s*, :cite:t:`stange-2006`) and calibration values for Southern
   California (*MLcan_hb*, :cite:t:`hutton-1987`).


Setup
=====

#. **Set the configuration and calibration parameters** in the global bindings
   similar
   to :ref:`global_ml`. Instead of configuring lots of global bindings profiles
   or station bindings one line per parameter can be added to the global module
   configuration (:file:`global.cfg`) which takes the form

   .. code-block:: params

      module.trunk.NET.STA.amplitude.MLcan.preFilter = value
      module.trunk.NET.STA.magnitude.MLcan.parametric.c0 = value

#. Add MLcan to the list of default amplitudes and magnitudes if MLcan is to be
   computed by automatic modules, e.g. of :ref:`scamp`, :ref:`scmag`.
#. Configure :ref:`scmag` (:confval:`magnitudes.average` in :file:`scmag.cfg`)
   for choosing the method to form the
   network magnitude from station magnitudes, e.g.

   .. code-block:: params

      magnitudes.average = MLcan:median

#. Add MLcan to the list of magnitudes preferred by :ref:`scevent`
   (:confval:`eventAssociation.magTypes` in :file:`scevent.cfg`) in order to let
   MLcan become the preferred magnitude.
#. Set defaults/visibility of MLcan in :term:`GUI` modules, e.g. :ref:`scolv`
   or :ref:`scesv`.
