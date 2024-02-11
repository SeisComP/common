The **mB**/**mB_BB** magnitude has been recommended by the IASPEI commission
(:cite:t:`bormann-2008`, :cite:t:`bormann-2009`, :cite:t:`iaspei-2013`).
It is based on amplitude measurements of body waves like :ref:`global_mb`, but
with the amplitude measured in a broad frequency range and longer time windows.
Instead of amplitude measurements on displacement data together with the
dominant period, the maximum velocity amplitude Vmax is taken directly from
velocity-proportional records with :math:`V = 2 \pi A/T`. The time window for
the measurement can be determined by the duration of the high-frequency (1-3 Hz)
radiation (:cite:t:`bormann-2008`). This time window usually contains the phases
P, pP, sP, PcP, but not PP. According to the long time window and broad
frequency range used for amplitude measurements mB saturates not like mb.

.. note::

   In |scname| the term **m_B** is a synonym for **mB_BB** which is used
   by IASPEI :cite:p:`iaspei-2013`.


Amplitude
---------

mB amplitudes are calculated on vertical-component displacement seismograms
in accordance with :cite:t:`bormann-2008` and similar to :ref:`mb <global_mb>`.
A default time window of 60 s is considered for amplitude measurements
at stations in the distance range of 5° to 105°.
If the epicentral is known, the length of the time window after the P wave onset
is

.. math::

   t = min(\Delta * 11.5, 60)

where :math:`\Delta` is the epicentral distance. The methods for measuring
amplitudes are configurable in the global bindings.


Station Magnitude
-----------------

The mB station magnitudes are calculated in accordance with :cite:t:`bormann-2008`.

.. math::

   mB = \log \left(\frac{A}{2\Pi}\right) + Q(h,\Delta) - 3.0

with A as the displacement amplitude in micrometers, T as the dominant period of
the signal in seconds, Q as a correction term for depth and distance.

* Amplitude unit in |scname|: **nanometers/s** (nm/s),
* Time window: 60 s if set by :ref:`scautopick`, otherwise 0 s - 11.5 * distance
  (deg) with 60 s minimum
* Default distance range: 5 - 105 deg, configurable: :confval:`magnitudes.mB.minDist`,
  :confval:`magnitudes.mB.maxDist`,
* Depth range: no limitation.

.. note::

   In 2013 the IASPEI commission (:cite:t:`iaspei-2013`) recommended a minimum
   distance of
   20 deg. However, the calibration formula (:cite:t:`bormann-2008`) which is
   integrated in
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
