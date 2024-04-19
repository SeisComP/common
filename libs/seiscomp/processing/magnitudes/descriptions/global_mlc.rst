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

* Name amplitude type: MLc.
* Expected unit of gain-corrected input data: m/s. Activate response correction
  in global bindings in case data are provided in acceleration.
* Components used for amplitude measurements: both horizontal components
  separately.

The parameters for measuring MLc amplitudes can be adjusted by global
binding parameters:

.. csv-table::
   :widths: 20 25 15 30
   :header: Topic, Parameter, Default, Comment
   :align: left
   :delim: ;

   :ref:`Filtering <filter-grammar>`; :confval:`amplitudes.MLc.preFilter`; :ref:`BW(3,0.5,12) <filter-bw>`; Applied before instrument simulation
   :term:`Wood-Anderson simulation <Wood-Anderson seismometer>`; :confval:`amplitudes.MLc.applyWoodAnderson`; true; When WA simulation is inactive, measured amplitudes take the units of the gain of the stream on which they were measured.
   :term:`Wood-Anderson parameters <Wood-Anderson seismometer>`; :confval:`amplitudes.WoodAnderson.gain`;see comment; Defaults are according to :ref:`IASPEI recommendation <concepts_magnitudes-wa>`.
   Amplitude scaling; :confval:`amplitudes.MLc.amplitudeScale`; 1; Apply for scaling measured amplitudes to units required by the magnitude
   Amplitude combination; :confval:`amplitudes.MLc.combiner`;max; Method for combining amplitudes measured on both horizontal components

Some additional parameters require you to create an amplitude-type profile for
global binding parameters. Name the profile like the amplitude name, hence MLc
replacing '$name' in the parameters below:

.. csv-table::
   :widths: 20 25 15 30
   :header: Topic, Parameter, Default, Comment
   :align: left
   :delim: ;

   Minimum distance; :confval:`amplitudes.$name.minDist`; 0;
   Maximum distance; :confval:`amplitudes.$name.maxDist`; 8; Cannot be extended beyond default
   Minimum source depth; :confval:`amplitudes.$name.minDepth`; 0; Can be negative
   Maximum source depth; :confval:`amplitudes.$name.maxDepth`; 80; Can be extended beyond default
   Noise window begin; :confval:`amplitudes.$name.noiseBegin`; -30;  (+++)
   Noise window end; :confval:`amplitudes.$name.noiseEnd`; -5;  (+++)
   Signal window begin; :confval:`amplitudes.$name.signalBegin`; -5;  (+++)
   Signal window end; :confval:`amplitudes.$name.signalEnd`; 150 (+) or distance/3+30 (++); (**+**) When measured by :ref:`scautopick`, (**++**) When measured by :ref:`scamp` or :ref:`scolv`  (+++)
   Minimum :term:`SNR`; :confval:`amplitudes.$name.minSNR`;not applied; Compares the maximum amplitudes measured within the signal and noise windows
   Amplitude staturation; :confval:`amplitudes.$name.saturationThreshold`; false; Apply for avoiding measurements on clipped data
   Response correction; :confval:`amplitudes.$name.enableResponses`; false; Activate for input units other than nm/s and set :confval:`amplitudes.$name.resp.minFreq`, :confval:`amplitudes.$name.resp.maxFreq`

**(+++)** All values defining the time windows for measuring noise and signal
are relative to P arrival time, read :ref:`Time grammar <time-grammar>`.

The Wood-Anderson simulation will convert input velocity data to ground
displacement in mm. The input data may be of a different unit after applying
:confval:`amplitudes.MLc.preFilter`, e.g. when integration is applied, and / or
when Wood-Anderson simulation is disabled. Configure
:confval:`amplitudes.MLc.amplitudeScale` for converting the unit of the
processed data to the unit expected by the
:ref:`station magnitude calibration <mlc_station_magnitude>` for the measured
amplitude.

.. note::

   * For comparing MLc amplitudes with :ref:`ML amplitudes <global_ml>` set the
     global bindings parameters

     .. code-block:: properties

        amplitudes.MLc.preFilter = ""
        amplitudes.MLc.combiner = average

   * The default values are taken from :cite:t:`stange-2006`.

.. _mlc_station_magnitude:

Station magnitudes
------------------

Station magnitudes are computed from measured amplitudes by applying a
configurable calibration function when the origin is within depths and distance
constraints. The parameters are configurable in global bindings or by global
module parameters when applying
:ref:`regionalization <concepts-magnitudes-regionalization>`.

Station corrections are configurable by a magnitude-type profile named MLc in
global bindings with :confval:`magnitudes.$name.offset` or the equivalent in
global module configuration as :confval:`module.trunk.NET.STA.magnitudes.MLc.offset`.
The latter is not supported by :ref:`scconfig` but it reduces the amount of
required bindings.

The calibration function is considered in one of the forms

* Parametric when :confval:`magnitudes.MLc.calibrationType` = "parametric"`:

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

  The default values are valid for SW-Germany (:cite:t:`stange-2006`), c6 and H
  have been added for supporting dependency on depth (:cite:t:`rhoades-2020`).

* log10(A0)-based non-parametric when :confval:`magnitudes.MLc.calibrationType` = "A0"`:

  .. math::

     MLc = \log_{10}(A) - \log_{10}(A_0)

  where

  * :math:`log_{10}(A_0)`: distance-dependent correction value. Read
    :ref:`global_mlv` for the details.

.. note::

   * The magnitude calibration function can be regionalized by adjusting global
     module configuration parameters in MLc region profiles of
     :confval:`magnitudes.MLc.region.*` and in a *MLc* Magnitude type profile
     e.g., in :file:`global.cfg`.
   * The default values for parametric calibration are taken from
     :cite:t:`stange-2006` and :cite:t:`rhoades-2020`.

Configurable parameters:

.. csv-table::
   :widths: 20 25 15 30
   :header: Topic, Parameter, Default, Comment
   :align: left
   :delim: ;

   Distance type; :confval:`magnitudes.MLc.distMode`; hypocentral; epicentral or hyocentral can be selected
   Minimum distance; :confval:`magnitudes.MLc.minDist`; -1;
   Maximum distance; :confval:`magnitudes.MLc.maxDist`; 8; Measurements beyond 8 deg are strictly ignored
   Minimum source depth; :confval:`magnitudes.MLc.minDepth`; -10;
   Maximum source depth; :confval:`magnitudes.MLc.maxDepth`; 80; Can be extended beyond default
   Amplitude type;; MLc; Configurable by :ref:`magnitude alias <concepts_magnitudes-aliases>`
   Amplitude unit;; mm; other units can be assumed by amplitude scaling with :confval:`amplitudes.MLc.amplitudeScale`
   Magnitude calibration type; :confval:`magnitudes.MLc.calibrationType`; parametric; parametric and A0 (non-parametric) are available
   Linear magnitude correction;:confval:`magnitudes.$name.multiplier`; 1.0; Configure station corrections more conveniently configurable in global module configuration as :confval:`module.trunk.NET.STA.magnitudes.MLc.multiplier`
   Constant magnitude correction;:confval:`magnitudes.$name.offset`; 0.0; Configure station corrections more conveniently configurable in global module configuration as :confval:`module.trunk.NET.STA.magnitudes.MLc.offset`
   ;;;
   **parametric** calibration;;; Parameters are used for :confval:`magnitudes.MLc.calibrationType` = parametric
   ;:confval:`magnitudes.MLc.parametric.c0`;0.0;
   ;:confval:`magnitudes.MLc.parametric.c1`;0.69;
   ;:confval:`magnitudes.MLc.parametric.c2`;0.00095;
   ;:confval:`magnitudes.MLc.parametric.c3`;1.11;
   ;:confval:`magnitudes.MLc.parametric.c4`;0.0;
   ;:confval:`magnitudes.MLc.parametric.c5`;1.0;
   ;:confval:`magnitudes.MLc.parametric.c6`;0.0; see :cite:t:`rhoades-2020`
   ;:confval:`magnitudes.MLc.parametric.H`;40.0; see :cite:t:`rhoades-2020`
   ;;;
   **non-parametric** calibration;;; Parameters are used for :confval:`magnitudes.MLc.calibrationType` = A0
   ;:confval:`magnitudes.MLc.A0.logA0`;0:-1.3,60:-2.8,100:-3.0,400:-4.5,1000:-5.85; from :ref:`ML magnitude <global_ml>`


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


Examples
========

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


Setup
=====

#. **Set the configuration and calibration parameters** in the global bindings
   similar
   to :ref:`global_ml`. Instead of configuring lots of global bindings profiles
   or station bindings one line per parameter can be added to the global module
   configuration (:file:`global.cfg`) which takes the form

   .. code-block:: properties

      module.trunk.NET.STA.amplitudes.MLc.preFilter = value
      module.trunk.NET.STA.magnitudes.MLc.parametric.c0 = value

#. Add MLc to the list of default amplitudes and magnitudes if MLc is to be
   computed by automatic modules, e.g. of :ref:`scamp`, :ref:`scmag`.
#. Configure :ref:`scmag` (:confval:`magnitudes.average` in :file:`scmag.cfg`)
   for choosing the method to form the
   network magnitude from station magnitudes, e.g.

   .. code-block:: properties

      magnitudes.average = MLc:median

#. Add MLc to the list of magnitudes preferred by :ref:`scevent`
   (:confval:`eventAssociation.magTypes` in :file:`scevent.cfg`) in order to let
   MLc become the preferred magnitude.
#. Set defaults/visibility of MLc in :term:`GUI` modules, e.g. :ref:`scolv`
   or :ref:`scesv`.
