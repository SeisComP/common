Ms_20 is the surface wave magnitude measured on the vertical component at around
20 s period	in accordance with the IASPEI standards. Please read the
specifications in :cite:t:`iaspei-2013`.


Amplitude
---------

The Ms_20 amplitudes are measured on vertical-component displacement seismograms
corrected for the instrument response of a :term:`WWSSN_LP` seismograph.
For configuration of amplitude measurement create and configure an
*Ms_20*-named amplitude-type profile in global bindings.
Bindings configuration parameters (replace *$name* by *Ms_20*):

* Time window: *max(OT+R/4,-5)* - *min(OT+R/3,3000)*, configurable:
  :confval:`amplitudes.$name.signalBegin` and
  :confval:`amplitudes.$name.signalEnd`.
* Period range: 18 s - 22 s, configurable: :confval:`amplitudes.$name.minPeriod`,
  :confval:`amplitudes.$name.maxPeriod`. Some agencies extend the range up to
  12 s - 28 s.
* Default distance range: 20 - 160 deg, configurable:
  :confval:`amplitudes.$name.minDist`, :confval:`amplitudes.$name.maxDist`,
* Depth range: <= 100 km, configurable:  :confval:`amplitudes.$name.minDepth`,
  :confval:`amplitudes.$name.maxDepth`,
* Signal-to-noise ration: 0, configurable:  :confval:`amplitudes.$name.minSNR`,
* Clipped seismograms: false, configurable:
  :confval:`amplitudes.$name.saturationThreshold`.

.. note::

   The bindings configuration parameters :confval:`magnitudes.Ms_20.minVelocity`
   and :confval:`magnitudes.Ms_20.maxVelocity` are obsolete, not considered
   anymore and replaced by :confval:`amplitudes.$name.signalBegin` and
   :confval:`amplitudes.$name.signalEnd`, respectively.


Station Magnitude
-----------------

Ms_20 is the surface-wave magnitude at 20 s period based on the recommendations
by the IASPEI magnitude working group issued on 27 March, 2013 (:cite:t:`iaspei-2013`).

.. math::

   Ms(20) = \log \left(\frac{A}{T}\right) + 1.66 \log(\Delta) + 0.3

with

* A: :term:`WWSSN_LP` corrected ground displacement in units of nm measured on
  the vertical-component seismogram as the maximum absolute trace amplitude of a
  surface wave at periods between 18 s and 22 s,
* T: period of the surface wave in seconds.

The term *Ms_20* is chosen in accordance with the IASPEI standard as of 2013
(:cite:t:`iaspei-2013`). Alternatively, the term *Ms(20)* may be used.
For configuration of magnitude computation create and configure an *Ms_20*-named
magnitude-type profile in global bindings.
Bindings configuration parameters (replace *$name* by *Ms_20*):

* Amplitude unit: nm, ground displacement corrected for :term:`WWSSN_LP`
* Period range: 18 s - 22 s, configurable: :confval:`magnitudes.$name.minPeriod`,
  :confval:`magnitudes.$name.maxPeriod`. Some agencies extend the range up to
  12 s - 28 s.
* Default distance range: 20 - 160 deg, configurable:
  :confval:`magnitudes.$name.minDist`, :confval:`magnitudes.$name.maxDist`
* Depth range: <= 100 km, configurable:  :confval:`magnitudes.$name.minDepth`,
  :confval:`magnitudes.$name.maxDepth`

.. note::

   The bindings configuration parameters :confval:`magnitudes.Ms_20.lowerPeriod`
   and :confval:`magnitudes.Ms_20.upperPeriod` are obsolete, not considered
   anymore and replace by :confval:`magnitudes.$name.minPeriod` and
   :confval:`magnitudes.$name.maxPeriod`, respectively.


Network magnitude
-----------------

The network magnitude is computed from station magnitudes automatically by
:ref:`scmag` or interactively by :ref:`scolv`.
The trimmed mean method is applied. Outliers beyond the outer 12.5% percentiles
are removed before forming the mean. The method can be adjusted in :ref:`scmag`
by :confval:`magnitudes.average`.


Setup
=====

#. Add and configure *Ms_20*-named amplitude and magnitude profiles in global
   bindings.
#. Add *Ms_20* to the list of measured amplitudes and computed magnitudes in
   the module configuration of :ref:`scamp`, :ref:`scmag` and :ref:`scolv`.
#. Configure the averaging method in :ref:`scmag`.
#. Configure the magnitude priority list in :ref:`scevent` in case you wish to
   give priority to Ms_20 for becoming the preferred magnitude of an event.
#. Add *Ms_20* the the list visible magnitudes in the module configuration of
   :ref:`scesv` or :ref:`scolv`. The magnitude will become visible in the
   summary view of the GUIs.
