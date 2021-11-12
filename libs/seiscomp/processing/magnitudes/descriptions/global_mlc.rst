The MLc magnitude is very similar to the original :ref:`ML<global_ml>`,
except that

* A parametric :ref:`magnitude calibration <mlc_station_magnitude>` function applies,
* Hypocentral distance is used by default.

MLc also provides additional flexibility by configuration of global bindings:

* Amplitudes can be pre-filtered before applying Wood-Anderson simulation
  (:confval:`amplitudes.MLc.preFilter`),
* Wood-Anderson simulation is optional (:confval:`amplitudes.MLc.applyWoodAnderson`),
* Measured amplitudes can be scaled accounting for expected unit
  (:confval:`amplitudes.MLc.amplitudeScale`),
* Consider either hypocentral or epicentral distance
  (:confval:`magnitudes.MLc.distMode`).


Station Amplitudes
------------------

The MLc amplitude calculation is very similar to the original :ref:`ML<global_ml>`,
except that they can be pre-filtered and applying Wood-Anderson simulation is
optional: :confval:`amplitudes.MLc.preFilter`, :confval:`amplitudes.MLc.applyWoodAnderson`.
By default amplitudes are measured on both horizontal components where the absolute
maxima are taken. They are combined to a final measured amplitude by taking the mean.
The methods for measuring and combining the amplitudes are configurable:
:confval:`amplitudes.MLc.measureType`, :confval:`amplitudes.MLc.combiner`.

The Wood-Anderson simulation will convert input velocity data to ground displacement
in mm. The input data may be of a different unit after applying
:confval:`amplitudes.MLc.preFilter`, e.g. when integration is applied, and / or
when Wood-Anderson simulation is disabled. Configure :confval:`amplitudes.MLc.amplitudeScale`
for converting the unit of the processed data to the unit expected by the
:ref:`station magnitude calibration <mlc_station_magnitude>` for the measured
amplitude.


.. _mlc_station_magnitude:

Station Magnitude
-----------------

MLc considers a parametric calibration function and hypocentral (default) or
epicentral distance :math:`r` as configurable by :confval:`magnitudes.MLc.distMode`.
For r <= :confval:`magnitudes.MLc.maxDist` individual station magnitudes
MLc are calculated as:

.. math::

   MLc = \log_{10}(A) + c_3 * \log_{10}(r/c_5) + c_2 * (r + c_4) + c_1 + c_0(station)

where

* A: displacement amplitude measured in unit of mm or as per configuration
* r: hypocentral (default) or epicentral distance
* c1, c2, c3, c4, c5: general calibration parameters
* c0: station-specific correction

The following conditions apply:

* Amplitude unit in SeisComP: **millimeter** (mm)
* Time window: 150 s by :ref:`scautopick` or distance dependent with :math:`endTime = distance [km]/ 3 + 30`
* Distance range: 0 - 8 deg (can be lowered)
* Depth range: 0 - 60 km (can be lowered)


Network Magnitude
-----------------

Originally the media was computed from all station MLc to form the
:term:`network magnitude` MLc. Here, the trimmed mean is applied. Outliers beyond the
outer 12.5% percentiles are removed before forming the mean. The method can be
adjusted in :ref:`scmag` by :confval:`magnitudes.average`.


Examples
--------

The flexibility of the amplitude and magnitude processing allows to apply MLc
in various use cases, e.g.

* Pre-filtered and gain-corrected amplitudes, Wood-Anderson corrected and
  measured in mm, e.g. Stange, 2006:

  .. math::

     MLc = \log_{10}(A) + 1.11 * \log_{10}(r) + 0.00095 * r + 0.69 + c_0

* Wood-Anderson-corrected displacement amplitudes measured in mm, e.g.
  Hutton and Boore, 1987, for southern California:

  .. math::

     MLc = \log_{10}(A) + 1.110 * \log_{10}(r / 100) + 0.00189 * (r - 100) + 3.0

* Pre-filtered velocity amplitudes in units of mym/s, no Wood-Anderson correction,
  e.g. Hiemer and Roessler, 2012:

  .. math::

     MLc = \log_{10}(A) - log_{10}(2\Pi) + 2.1 * \log_{10}(r) - 1.7 + c_0


Configuration
=============

#. Set the configuration and calibration parameters in the global bindings similar
   to :ref:`global_ml`. Instead of configuring lots of global bindings profiles or
   station bindings one line per parameter can be added to the global module
   configuration (:file:`global.cfg`) which takes the form ::

      module.trunk.NET.STA.amplitude.MLc.preFilter = value
      module.trunk.NET.STA.magnitude.MLc.c0 = value

#. Add MLc to the list of default amplitudes and magnitudes if MLc is to be
   computed by automatic modules, e.g. of :ref:`scamp`, :ref:`scmag`.
#. Set defaults / visibility of MLc in :ref:`scolv` and in :ref:`scesv`.
#. Configure :ref:`scmag` (:file:`scmag.cfg`) for choosing the method to form the
   network magnitude from station magnitudes, e.g. ::

      magnitudes.average = MLc:median

.. note ::

   All default values for bindings configuration values are from Stange, 2006.


References
==========

* S. Stange (2006). ML determination for local and regional events using a sparse
  network in Southwestern Germany. J. Seismology, 10:247–257. DOI: 10.1007/s10950-006-9010-6
* Hutton, L.K. and D.M. Boore (1987). The ML scale in southern California,
  Bull. Seismol. Soc. Am. 77, 2074–2094.
* S. Hiemer and D. Roessler (2012). Monitoring the West Bohemian earthquake swarm
  in 2008/2009 by a temporary small-aperture seismic array. J. Seismology, 16:169–182,
  J Seismol (2012) 16:169–182. DOI: 10.1007/s10950-011-9256-5
