mb_Lg is the custom magnitude for local events derived from the ML plugin.
The implementation is based on specifications by Spanish National Geographic Institute,
based on López, C. (2008). Nuevas fórmulas de magnitud para la Península Ibérica y su entorno. 
Trabajo de investigación del Máster en Geofísica y Meteorología. 
Departamento de Física de la Tierra, Astronomía y Astrofísica 
Universidad Complutense de Madrid. Madrid.


The mb_Lg magnitude is very similar to the original :ref:`ML<global_ml>`,
except that by default

* Amplitude pre-filtering is applied.
* A parametric :ref:`magnitude calibration <mlc_station_magnitude>` function
  applies.
* Hypocentral distance is used.

Regionalization of magnitude computation is provided through global module
configuration.
Configuration of global bindings provides additional flexibility:

* Amplitudes can be pre-filtered before simulating a :term:`Wood-Anderson seismometer`
  (:confval:`amplitudes.mb_Lg.preFilter`),
* Wood-Anderson simulation is optional:
  :confval:`amplitudes.mb_Lg.applyWoodAnderson`,
* Measured amplitudes can be scaled accounting for expected unit
  (:confval:`amplitudes.mb_Lg.amplitudeScale`),
* A parametric or A0-based non-parametric
  :ref:`magnitude calibration <mlc_station_magnitude>`
  function can be applied as controlled by
  :confval:`magnitudes.mb_Lg.calibrationType`.
* Consider either hypocentral or epicentral distance for computing magnitudes
  (:confval:`magnitudes.mb_Lg.distMode`).

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
mb_Lg amplitudes can be measured automatically by :ref:`scautopick` or :ref:`scamp`
or interactively by :ref:`scolv` very similarly to the original :ref:`ML<global_ml>`,
except that they can be pre-filtered and simulation of a :term:`Wood-Anderson seismometer`
is optional: :confval:`amplitudes.mb_Lg.preFilter`,
:confval:`amplitudes.mb_Lg.applyWoodAnderson`.
By default amplitudes are measured on both horizontal components where the
absolute maxima are taken. They are combined to a final measured amplitude by
taking the mean. The methods for measuring and combining the amplitudes are
configurable:
:confval:`amplitudes.mb_Lg.measureType`, :confval:`amplitudes.mb_Lg.combiner`.

The Wood-Anderson simulation will convert input velocity data to ground
displacement in mm. The input data may be of a different unit after applying
:confval:`amplitudes.mb_Lg.preFilter`, e.g. when integration is applied, and / or
when Wood-Anderson simulation is disabled. Configure
:confval:`amplitudes.mb_Lg.amplitudeScale` for converting the unit of the
processed data to the unit expected by the
:ref:`station magnitude calibration <mlc_station_magnitude>` for the measured
amplitude.

.. note::

   For comparing mb_Lg amplitudes with :ref:`ML amplitudes <global_ml>` set the
   global bindings parameters ::

      amplitudes.mb_Lg.preFilter = ""
      amplitudes.mb_Lg.combiner = average


.. _mlc_station_magnitude:

Station Magnitudes
------------------

Station magnitudes are computed from measured amplitudes automatically by
:ref:`scmag`
or interactively by :ref:`scolv`. By global bindings configuration mb_Lg considers

* Hypocentral (default) or epicentral distance: :confval:`magnitudes.mb_Lg.distMode`.
* Distance range: :confval:`magnitudes.mb_Lg.minDist`, :confval:`magnitudes.mb_Lg.maxDist`.
* Events with depth up to :confval:`magnitudes.mb_Lg.maxDepth`.
* Parametric or non-parametric calibration functions

  * parametric when :confval:`magnitudes.mb_Lg.calibrationType` = "parametric"`:

    .. math::

       mb_Lg = \log_{10}(A/(2^PI)) + c_0 * \log_{10}(r) + c_1 * (r) + c_2)

    where

    * *A*: displacement amplitude measured in unit of mm or as per configuration 
    * *r*: hypocentral (default) or epicentral distance
    * *c0*, *c1*, *c2*: general calibration parameters
    * *r*: Hypocentral (default) or epicentral distance as configured by
      :confval:`magnitudes.mb_Lg.distMode`.
    * *PI*: The value of Pi number (3.14159265359) 

  * A0-based non-parametric when :confval:`magnitudes.mb_Lg.calibrationType` = "A0"`:

    .. math::

       mb_Lg = \log_{10}(A) - \log_{10}(A_0)

    where

    * :math:`log_{10}(A_0)`: distance-dependent correction value. Read
      :ref:`global_mlv` for the details.

.. note::

   The magnitude calibration function can regionalized by adjusting global module
   configuration parameters in mb_Lg region profiles of
   :confval:`magnitudes.mb_Lg.region.*` and in a *mb_Lg* Magnitude type profile e.g.
   in :file:`global.cfg`.


Network Magnitude
-----------------

The network magnitude is computed from station magnitudes automatically by
:ref:`scmag` or interactively by :ref:`scolv`.
Originally the median was computed from all station mb_Lg to form the
:term:`network magnitude` mb_Lg. Here, the trimmed mean is applied. Outliers
beyond the outer 12.5% percentiles are removed before forming the mean. The
method can be adjusted in :ref:`scmag` by :confval:`magnitudes.average`.



Setup
=====

#. **Set the configuration and calibration parameters** in the global bindings
   similar
   to :ref:`global_ml`. Instead of configuring lots of global bindings profiles
   or station bindings one line per parameter can be added to the global module
   configuration (:file:`global.cfg`) which takes the form

   .. code-block:: params

      module.trunk.NET.STA.amplitude.mb_Lg.preFilter = value
      module.trunk.NET.STA.magnitude.mb_Lg.parametric.c0 = value

#. Add mb_Lg to the list of default amplitudes and magnitudes if mb_Lg is to be
   computed by automatic modules, e.g. of :ref:`scamp`, :ref:`scmag`.
#. Configure :ref:`scmag` (:confval:`magnitudes.average` in :file:`scmag.cfg`)
   for choosing the method to form the
   network magnitude from station magnitudes, e.g.

   .. code-block:: params

      magnitudes.average = mb_Lg:median

#. Add mb_Lg to the list of magnitudes preferred by :ref:`scevent`
   (:confval:`eventAssociation.magTypes` in :file:`scevent.cfg`) in order to let
   mb_Lg become the preferred magnitude.
#. Set defaults/visibility of mb_Lg in :term:`GUI` modules, e.g. :ref:`scolv`
   or :ref:`scesv`.
