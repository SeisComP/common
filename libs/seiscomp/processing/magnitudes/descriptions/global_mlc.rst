MLc is a custom magnitude for local events based on :ref:`ML<global_ml>` but
with greater flexibility.
The original implementation is based on specifications by the Hessian Agency for
Nature Conservation, Environment and Geology, Hessian Earthquake Service.
More options have been added allowing the magnitude to be configured with
great flexibility in order to account for many different conditions. The
general procedures for measuring amplitudes and computing magnitudes are
outlined in the :ref:`"Concepts" section on magnitudes <concepts_magnitudes>`.

The MLc magnitude is very similar to the original :ref:`ML<global_ml>`,
except that

* Amplitude pre-filtering is applied by default.
* Wood-Anderson simulation is optionally applied and can be deactivated.
* Measured amplitudes can be scaled accounting for expected units.
* Measured amplitudes are combined by taking the maximum instead of the average.
* A parametric :ref:`magnitude calibration <mlc_station_magnitude>` function
  applies but a correction in the form log10(A0) can be configured for converting
  measured amplitudes to station magnitudes.
* Hypocentral instead of epicentral distance is considered by default.


Amplitudes
----------

Some general conditions apply for measuring amplitudes:

* Measured amplitude type: MLc.
* Expected unit of gain-corrected input data: m/s. Activate response correction
  in global bindings in case data are provided in acceleration.
* Components used for amplitude measurements: both horizontal components
  separately.

The default parameters for measuring MLc amplitudes can be adjusted by global
binding parameters:

* Filtering before instrument simulation: :ref:`BW(3,0.5,12) <filter-bw>`,
  configurable by :confval:`amplitudes.MLc.preFilter`.
* :term:`Wood-Anderson seismometer` simulation: yes, can be deactivated by
  :confval:`amplitudes.MLc.applyWoodAnderson`.
* Characteristics of :term:`Wood-Anderson seismometer`: according to IASPEI
  recommendations. Can be adjusted by :confval:`amplitudes.WoodAnderson.gain`,
  :confval:`amplitudes.WoodAnderson.T0`, :confval:`amplitudes.WoodAnderson.h`
  in global bindings or global module configuration.
* Amplitude scaling: 1, configure by :confval:`amplitudes.MLc.amplitudeScale`
  for considering non-default units by magnitude.
* Method applied for measuring amplitudes: absolute maximum, configurable in
  global bindings by :confval:`amplitudes.MLc.measureType`.
* Method for combining amplitude measurements: mean, configurable in global
  bindings by :confval:`amplitudes.MLc.combiner`.

Some additional parameters require you to create an amplitude-type profile for
global binding parameters. Name the profile like the amplitude name, hence MLc:

* Time window for measuring signal amplitudes [s]: P pick time + 150 s by
  :ref:`scautopick` or distance [km]/3 km/s + 30 s,
  the relevant parameters are: :confval:`amplitudes.MLc.signalBegin`,
  :confval:`amplitudes.MLc.signalEnd`. :ref:`Time grammar <time-grammar>` may be
  applied for begin and end times.
* Time window for measuring noise amplitudes [s]: 30 s before the P pick,
  the relevant parameters are: :confval:`amplitudes.MLc.noiseBegin`,
  :confval:`amplitudes.MLc.noiseEnd`. :ref:`Time grammar <time-grammar>` may be
  applied for begin and end times.
* Minimum SNR: 0, configurable by :confval:`amplitudes.MLc.minSNR`.
* Distance range: 0 - 8 deg, configurable by :confval:`amplitudes.MLc.minDist`,
  :confval:`amplitudes.MLc.maxDist`, stations at distances beyond 8 deg will be strictly
  ignored.
* Depth range: <= 80 km, can be adjusted and extended by
  :confval:`amplitudes.MLc.minDepth` and :confval:`amplitudes.MLc.maxDepth`.

Most parameters controlling the amplitude measurements are configurable in
global bindings or global module configuration.

The Wood-Anderson simulation will convert input velocity data to ground
displacement in mm. The input data may be of a different unit after applying
:confval:`amplitudes.MLc.preFilter`, e.g. when integration is applied, and / or
when Wood-Anderson simulation is disabled. Configure
:confval:`amplitudes.MLc.amplitudeScale` for converting the unit of the
processed data to the unit expected by the
:ref:`station magnitude calibration <mlc_station_magnitude>` for the measured
amplitude.

.. note::

   For comparing MLc amplitudes with :ref:`ML amplitudes <global_ml>` set the
   global bindings parameters ::

      amplitudes.MLc.preFilter = ""
      amplitudes.MLc.combiner = average


.. _mlc_station_magnitude:

Station magnitudes
------------------

Default properties, most parameters are configurable in global bindings:

* Distance type: hypocentral, epicentral can be selected by :confval:`magnitudes.MLc.distMode`.
* Distance range: 0 - 8 deg, configurable by :confval:`magnitudes.MLc.minDist`,
  :confval:`magnitudes.MLc.maxDist`, measurements beyond 8 deg will be strictly
  ignored.
* Depth range: <= 80 km, can be extended by :confval:`magnitudes.MLc.maxDepth`.
* Expected amplitude type: MLc, configurable by magnitude alias.
* Expected amplitude unit: millimeter (mm), other units can be assumed by
  amplitude scaling with :confval:`amplitudes.MLc.amplitudeScale`.
* Magnitude calibration type: parametric, parametric and non-parametric are
  available through :confval:`magnitudes.MLc.calibrationType`.
* Calibration function (see below for the equations): default values are valid
  for SW-Germany (:cite:t:`stange-2006`), configurable by global bindings
  depending on the actual calibration type:

  * parametric: :confval:`magnitudes.MLc.parametric.c0`,
    :confval:`magnitudes.MLc.parametric.c1`,
    :confval:`magnitudes.MLc.parametric.c2`,
    :confval:`magnitudes.MLc.parametric.c3`,
    :confval:`magnitudes.MLc.parametric.c4`,
    :confval:`magnitudes.MLc.parametric.c5`,
    :confval:`magnitudes.MLc.parametric.c6`,
    :confval:`magnitudes.MLc.parametric.H`

  * A0: :confval:`magnitudes.MLc.A0.logA0`
* Station correction: none, configurable by a magnitude-type profile in global
  bindings with :confval:`magnitudes.MLc.offset` or the equivalent in global
  module configuration as :confval:`module.trunk.NET.STA.magnitudes.MLc.offset`.
  The latter is not supported by :ref:`scconfig` but it reduces the amount of
  required bindings.

The calibration function is considered in one of the forms

* parametric when :confval:`magnitudes.MLc.calibrationType` = "parametric"`:

  .. math::

     MLc = \log_{10}(A) + c_6 * h + c_3 * \log_{10}(r/c_5) + c_2 * (r + c_4) + c_1 + c_0(station)

  where

  * *A*: displacement amplitude measured in unit of mm or as per configuration
  * *r*: hypocentral (default) or epicentral distance
  * *c1*, *c2*, *c3*, *c4*, *c5*, *c6*: general calibration parameters
  * *c0*: station-specific correction
  * *r*: Hypocentral (default) or epicentral distance as configured by
    :confval:`magnitudes.MLc.distMode`
  * h: (source depth - :confval:`magnitudes.MLc.parametric.H`) when
    source depth > :confval:`magnitudes.MLc.parametric.H` but 0 otherwise.

* log10(A0)-based non-parametric when :confval:`magnitudes.MLc.calibrationType` = "A0"`:

  .. math::

     MLc = \log_{10}(A) - \log_{10}(A_0)

  where

  * :math:`log_{10}(A_0)`: distance-dependent correction value. Read
    :ref:`global_mlv` for the details.

.. note::

   The magnitude calibration function can be regionalized by adjusting global
   module configuration parameters in MLc region profiles of
   :confval:`magnitudes.MLc.region.*` and in a *MLc* Magnitude type profile e.g.
   in :file:`global.cfg`.

The flexibility of the amplitude and magnitude processing allows for MLc to be
applied in various use cases. Examples are given below.

* **Default:** Pre-filtered and gain-corrected amplitudes, Wood-Anderson
  corrected and measured in mm for Southwestern Germany, :cite:t:`stange-2006`:

  .. math::

     MLc = \log_{10}(A) + 1.11 * \log_{10}(r) + 0.00095 * r + 0.69 + c_0

* Wood-Anderson-corrected displacement amplitudes measured in mm for
  Southern California, :cite:t:`hutton-1987`:

  .. math::

     MLc = \log_{10}(A) + 1.110 * \log_{10}(r / 100) + 0.00189 * (r - 100) + 3.0

* Pre-filtered velocity amplitudes in units of mym/s (requiring to set
  :confval:`amplitudes.MLc.amplitudeScale`), no Wood-Anderson correction,
  for West Bohemia, e.g. :cite:t:`hiemer-2012`:

  .. math::

     MLc = \log_{10}(A) - log_{10}(2\Pi) + 2.1 * \log_{10}(r) - 1.7 + c_0

.. figure:: media/magnitude-calibrations_MLc_s_MLc_hb.png
   :align: center
   :width: 18cm

   MLc magnitudes for measured amplitude of 1 mm with default magnitude
   calibration (*MLc_s*, :cite:t:`stange-2006`) and calibration values for Southern
   California (*MLc_hb*, :cite:t:`hutton-1987`).


Network magnitude
-----------------

The network magnitude is computed from station magnitudes automatically by
:ref:`scmag` or interactively by :ref:`scolv`.
Originally the median was computed from all station MLc to form the
:term:`network magnitude` MLc. Here, the trimmed mean is applied. Outliers
beyond the outer 12.5% percentiles are removed before forming the mean. The
method can be adjusted in :ref:`scmag` by :confval:`magnitudes.average`.


Moment magnitude
----------------

MLc can be scaled to a moment magnitude, Mw(MLc), by a magnitude-type profile in
global module configuration. Read the
:ref:`Tutorial on moment magnitudes <tutorials_mags_moment>` for the details.


Magnitude aliases
-----------------

Magnitude aliases can be created by :confval:`magnitudes.aliases` in
global module configuration in order to derive other magnitude types from
original amplitudes and magnitudes. The actual amplitude and magnitude
parameters of the aliases will be configured in global bindings or by
magnitude-type profiles in global module configuration. Read the
:ref:`Tutorial on magnitude aliases <tutorials_magnitude-aliases>` for the
details.



Regionalization
---------------

Regionalization may be achieved by a magnitude-type profile in global module
configuration. Read the
:ref:`Tutorial on regionalization <tutorials_magnitude-region>` for the details.


Setup
=====

#. **Set the configuration and calibration parameters** in the global bindings
   similar
   to :ref:`global_ml`. Instead of configuring lots of global bindings profiles
   or station bindings one line per parameter can be added to the global module
   configuration (:file:`global.cfg`) which takes the form

   .. code-block:: params

      module.trunk.NET.STA.amplitudes.MLc.preFilter = value
      module.trunk.NET.STA.magnitudes.MLc.parametric.c0 = value

#. Add MLc to the list of default amplitudes and magnitudes if MLc is to be
   computed by automatic modules, e.g. of :ref:`scamp`, :ref:`scmag`.
#. Configure :ref:`scmag` (:confval:`magnitudes.average` in :file:`scmag.cfg`)
   for choosing the method to form the
   network magnitude from station magnitudes, e.g.

   .. code-block:: params

      magnitudes.average = MLc:median

#. Add MLc to the list of magnitudes preferred by :ref:`scevent`
   (:confval:`eventAssociation.magTypes` in :file:`scevent.cfg`) in order to let
   MLc become the preferred magnitude.
#. Set defaults/visibility of MLc in :term:`GUI` modules, e.g. :ref:`scolv`
   or :ref:`scesv`.

.. note::

   All default values for bindings configuration parameters are from
   :cite:t:`stange-2006`.
