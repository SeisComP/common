The name **mB** is used in |scname| as a synonym for **mB_BB** which is recommended
by the IASPEI commission [#iaspei2013]_ .

mB is a magnitude based on body waves like :ref:`global_mb`, but with the amplitude
measured in a broad
frequency range and longer time windows. Instead of amplitude measurements on displacement
data together with the dominant period, the maximum velocity amplitude Vmax is taken
directly from velocity-proportional records with :math:`V = 2 \pi A/T`. The time window for the
measurement can be determined by the duration of the high-frequency (1-3 Hz) radiation
(Bormann & Saul, 2008). This time window usually contains the phases P, pP, sP, PcP, but
not PP. According to the long time window and broad frequency range used for amplitude
measurements mB saturates not like mb.


Amplitude
---------

The mB amplitudes are calculated on vertical-component displacement seismograms
in accordance with [#bormann2008]_ and similar to :ref:`mb <global_mb>`.
A default time window of 60 s is considered for amplitude measurements
at stations in the distance range of 5° to 105°.
If the epicentral is known, the length of the time window after the P wave onset is

.. math::

   t = min(11.5 \Delta, 60)

where :math:`\Delta` is the epicentral distance. The methods for measuring
amplitudes are configurable in the global bindings.


Station Magnitude
-----------------

The mB station magntiudes are calculated in accordance with Bormann and Saul (2008) [#bormann2008]_.

.. math::

   mB = \log \left(\frac{A}{2\Pi}\right) + Q(h,\Delta) - 3.0

with A as the displacement amplitude in micrometers, T as the dominant period of the signal in seconds, Q as a correction term for depth and distance.

* Amplitude unit in |scname|: **nanometers/s** (nm/s),
* Time window: 60 s if set by :ref:`scautopick`, otherwise 0 s - 11.5 * distance (deg) with 60 s minimum
* Default distance range: 5 - 105 deg, configurable: :confval:`magnitudes.mB.minDist`,
  :confval:`magnitudes.mB.maxDist`,
* Depth range: no limitation.

.. note::

   In 2013 the IASPEI commission [#iaspei2013]_ recommended a minimum distance of
   20 deg. However, the calibration formula [#bormann2008]_ which is intergrated in
   |scname| allows the extension down to 5 deg while maintaining consistent magnitudes
   at 20 deg and beyond. Therefore, 5 deg is used as the default in
   :confval:`magnitudes.mB.minDist`.


Network magnitude
-----------------

By default, the trimmed mean is calculated from the station magnitudes to form
the :term:`network magnitude`. Outliers beyond the outer 12.5% percentiles are
removed before forming the mean.


Configuration
-------------

Adjust the configurable parameters in global bindings in the mB section or use
:file:`global.cfg`
as :ref:`global_mlv`. Add mB to the list of computed amplitudes and magnitudes
in the configuration of
:ref:`scamp` and :ref:`scmag` and in :ref:`scesv` or :ref:`scolv` for visibility.


References
==========

.. target-notes::

.. [#bormann2008]  P. Bormann and J. Saul (2008): The New IASPEI Standard
   Broadband Magnitude mB, Seismol. Res. Lett., 79 (5): 698–705, doi:
   https://doi.org/10.1785/gssrl.79.5.698

.. [#iaspei2013] IASPEI  magnitude working group (2013).
   SUMMARY OF MAGNITUDE WORKING GROUP RECOMMENDATIONS ON
   STANDARD PROCEDURES FOR DETERMINING EARTHQUAKE MAGNITUDES FROM DIGITAL DATA,
   `Link to PDF document
   <http://www.iaspei.org/commissions/commission-on-seismological-observation-and-interpretation/Summary_WG_recommendations_20130327.pdf>`_
