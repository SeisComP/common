.. _filterpicker:

===========
FilterPicker
===========

.. contents::
   :local:
   :depth: 3

Overview
========

**FilterPicker** is a robust, broadband phase detector and picker algorithm for real-time seismic monitoring and earthquake early-warning systems. The implementation is based on the work of Lomax et al. (2012) and operates on multiple frequency bands using characteristic functions to detect and pick seismic phases.

The module provides a primary picker for P-phase detection and picking, designed for real-time operation in the SeisComP :program:`scautopick` module.


Algorithm Description
=====================

The FilterPicker algorithm works through the following stages:

1. **Filter Bank Decomposition**
   
   Input data is decomposed through a bank of bandpass filters with logarithmically spaced center frequencies. This broadband approach ensures sensitivity to seismic phases across a wide frequency range.

2. **Characteristic Function Computation**
   
   For each frequency band, a characteristic function (CF) is computed based on the envelope STA/LTA (Short-Term Average / Long-Term Average) ratio. The CF emphasizes the onset of seismic phases.

3. **Integration**
   
   The maximum CF across all frequency bands is integrated over a configurable time window to enhance the detection signal.

4. **Detection and Picking**
   
   A pick is declared when the characteristic function exceeds an adaptive threshold. The exact onset time is determined from the CF shape.

5. **Uncertainty Estimation**
   
   Realistic timing uncertainty estimates are provided based on the rise time of the characteristic function.

6. **Polarity Determination**
   
   The onset polarity (positive, negative, or undecidable) is determined from the initial motion of the seismic signal.


Installation
============

The FilterPicker module is part of the SeisComP base processing library and is automatically included when building SeisComP from source.

Build Requirements
------------------

- SeisComP source code (scsrc)
- CMake >= 3.10
- C++ compiler with C++11 support

Build Instructions
------------------

1. The FilterPicker module is located in::

      scsrc/src/base/common/libs/seiscomp/processing/filterpicker/

2. Build SeisComP as usual::

      cd scsrc/build
      cmake ..
      make -j4
      make install

3. Verify the plugin is available::

      seiscomp exec scautopick --help


Configuration
=============

Basic Setup
-----------

To enable FilterPicker in :program:`scautopick`, add the following to your ``scautopick.cfg`` or profile configuration file:

.. code-block:: ini

   # Enable FilterPicker for P-phase picking
   picker = FilterPicker


Module Parameters
=================

.. table:: FilterPicker configuration parameters

   ========================= ========= ================================================
   Parameter                 Default   Description
   ========================= ========= ================================================
   ``picker.FilterPicker.numBands``       5         Number of frequency bands in the filter bank
   ``picker.FilterPicker.minFreq``        0.5       Minimum frequency in Hz
   ``picker.FilterPicker.maxFreq``        20.0      Maximum frequency in Hz
   ``picker.FilterPicker.windowSize``     2         Integration window size in seconds
   ``picker.FilterPicker.threshold``      2.0       Detection threshold (CF ratio)
   ``picker.FilterPicker.thresholdOff``   1.5       Reset threshold for detector
   ``picker.FilterPicker.adaptiveThreshold`` ``true`` Enable adaptive thresholding based on noise
   ``picker.FilterPicker.noiseBegin``     -10.0     Start of noise window relative to trigger (s)
   ``picker.FilterPicker.signalBegin``    -5.0      Start of signal window relative to trigger (s)
   ``picker.FilterPicker.signalEnd``      20.0      End of signal window relative to trigger (s)
   ``picker.FilterPicker.minSNR``         2.0       Minimum SNR required for valid pick
   ========================= ========= ================================================


Parameter Descriptions
======================

Frequency Bands (``numBands``)
------------------------------

The number of frequency bands determines how many parallel filters are applied to the input data. More bands provide better frequency coverage but increase computational cost.

- **Recommended range**: 4–8 bands
- **Default**: 5 bands
- **Trade-off**: More bands = better detection across frequencies, but higher CPU usage


Frequency Range (``minFreq``, ``maxFreq``)
------------------------------------------

Defines the frequency range covered by the filter bank. The bands are logarithmically spaced between these limits.

- **Typical**: 0.5–20 Hz (broadband)
- **Local/regional events**: Consider 1–30 Hz
- **Teleseismic events**: Consider 0.1–5 Hz


Detection Threshold (``threshold``)
-----------------------------------

The characteristic function ratio required to trigger a pick. Lower values increase sensitivity but may produce more false picks.

- **Lower values** (1.5–2.0): More sensitive, suitable for quiet stations
- **Higher values** (2.5–3.5): More conservative, suitable for noisy environments
- **Default**: 2.0


Adaptive Thresholding (``adaptiveThreshold``)
---------------------------------------------

When enabled, the detection threshold is automatically scaled based on the background noise level. This improves performance in varying noise conditions.

- **Enabled (true)**: Recommended for most installations
- **Disabled (false)**: Use fixed threshold regardless of noise


Integration Window (``windowSize``)
-----------------------------------

Time window for characteristic function integration. Longer windows provide more stable detection but increase latency.

- **Typical**: 2 seconds (default)
- **Fast response**: Reduce to 1 second
- **Noisy environments**: Increase to 3–5 seconds


SNR Threshold (``minSNR``)
--------------------------

Minimum signal-to-noise ratio required for a valid pick. Picks with SNR below this threshold are rejected.

- **Lower values** (1.5–2.0): Accept more picks, including weaker signals
- **Higher values** (3.0–5.0): Only accept high-quality picks
- **Default**: 2.0


Example Configurations
======================

Standard Configuration
----------------------

A balanced configuration suitable for most regional seismic networks:

.. code-block:: ini

   picker = FilterPicker

   picker.FilterPicker.numBands = 5
   picker.FilterPicker.minFreq = 0.5
   picker.FilterPicker.maxFreq = 20.0
   picker.FilterPicker.windowSize = 2
   picker.FilterPicker.threshold = 2.0
   picker.FilterPicker.adaptiveThreshold = true
   picker.FilterPicker.minSNR = 2.0


Local/Regional Events (Sensitive)
---------------------------------

Optimized for detecting local and regional events with higher frequency content:

.. code-block:: ini

   picker.FilterPicker.numBands = 6
   picker.FilterPicker.minFreq = 1.0
   picker.FilterPicker.maxFreq = 30.0
   picker.FilterPicker.threshold = 1.5
   picker.FilterPicker.minSNR = 1.5
   picker.FilterPicker.adaptiveThreshold = true


Teleseismic Events (Conservative)
---------------------------------

Optimized for teleseismic events with lower frequency content:

.. code-block:: ini

   picker.FilterPicker.numBands = 4
   picker.FilterPicker.minFreq = 0.1
   picker.FilterPicker.maxFreq = 5.0
   picker.FilterPicker.windowSize = 3
   picker.FilterPicker.threshold = 2.5
   picker.FilterPicker.minSNR = 3.0


Noisy Environment
-----------------

Configuration for stations with high background noise:

.. code-block:: ini

   picker.FilterPicker.numBands = 5
   picker.FilterPicker.threshold = 3.0
   picker.FilterPicker.minSNR = 3.5
   picker.FilterPicker.adaptiveThreshold = true
   picker.FilterPicker.windowSize = 3


Usage
=====

Real-time Processing
--------------------

Enable FilterPicker in your SeisComP configuration and start :program:`scautopick`:

.. code-block:: bash

   seiscomp start scautopick

Or run with command-line configuration override:

.. code-block:: bash

   seiscomp exec scautopick --config-filter="picker=FilterPicker"


Offline Processing
------------------

Process archived data with FilterPicker:

.. code-block:: bash

   seiscomp exec scautopick --offline --playback -I data.mseed --ep


Testing and Debugging
---------------------

Test FilterPicker with specific parameters:

.. code-block:: bash

   seiscomp exec scautopick \
     --config-filter="picker.FilterPicker.numBands=6" \
     --config-filter="picker.FilterPicker.threshold=2.5" \
     --debug


Performance Considerations
==========================

The FilterPicker is designed for real-time operation. Performance depends on several factors:

Computational Cost
------------------

- **Number of frequency bands**: Each additional band requires one bandpass filter operation
- **Sampling rate**: Higher sampling rates increase the number of samples to process
- **Integration window size**: Larger windows require more memory but have minimal CPU impact

Typical Performance
-------------------

On modern hardware (as of 2024):

- 100 Hz data, 5 bands: < 10 ms per trace per second
- Memory usage: ~1 MB per active trace

Optimization Tips
-----------------

1. Use the minimum number of frequency bands necessary for your application
2. For high sampling rate data (>100 Hz), consider decimating before picking
3. Enable adaptive thresholding to reduce false picks in varying noise conditions
4. Adjust the frequency range to match your expected signal characteristics


Tuning Guidelines
=================

For Local/Regional Events
-------------------------

- Increase ``maxFreq`` to 30–50 Hz
- Use more frequency bands (6–8)
- Lower the threshold (1.5–2.0)
- Reduce ``minSNR`` to 1.5–2.0

For Teleseismic Events
----------------------

- Decrease ``minFreq`` to 0.1–0.2 Hz
- Use fewer frequency bands (3–4)
- Increase ``windowSize`` to 3–5 seconds
- Increase threshold to 2.5–3.5

For Noisy Environments
----------------------

- Enable adaptive thresholding (``adaptiveThreshold = true``)
- Increase the threshold to 3.0–4.0
- Increase ``minSNR`` to 3.0–5.0
- Consider increasing ``windowSize`` for more stable detection

For Earthquake Early Warning (EEW)
----------------------------------

- Minimize ``windowSize`` (1–2 seconds) for fastest response
- Use moderate threshold (2.0–2.5) to balance speed and reliability
- Enable adaptive thresholding for varying noise conditions
- Consider reducing ``numBands`` to 4 for faster processing


Output
======

Pick Properties
---------------

FilterPicker generates picks with the following properties:

- **Time**: Onset time with sub-sample precision
- **Uncertainty**: Timing uncertainty estimated from CF rise time (typically 0.1–1.0 s)
- **SNR**: Signal-to-noise ratio of the detected phase
- **Polarity**: Initial motion polarity (positive, negative, or undecidable)
- **Method ID**: ``FilterPicker``
- **Filter ID**: Description of the filter bank configuration used


Quality Indicators
------------------

The following indicators can be used to assess pick quality:

- **SNR**: Higher values (>5) indicate confident picks
- **Uncertainty**: Lower values indicate sharper onsets
- **Polarity**: Confident polarity determination suggests good signal quality


Troubleshooting
===============

No Picks Generated
------------------

1. **Check threshold**: Lower the ``threshold`` parameter
2. **Check frequency range**: Ensure ``minFreq`` and ``maxFreq`` match your signal
3. **Check SNR threshold**: Lower ``minSNR`` if picks are being rejected
4. **Verify data quality**: Check for gaps, spikes, or other data issues

Too Many False Picks
--------------------

1. **Increase threshold**: Raise the ``threshold`` parameter
2. **Increase SNR requirement**: Raise ``minSNR``
3. **Enable adaptive thresholding**: Set ``adaptiveThreshold = true``
4. **Adjust frequency range**: Narrow the band to exclude noise frequencies

Picks Have Large Uncertainty
----------------------------

1. **Check signal quality**: Noisy signals produce uncertain picks
2. **Reduce integration window**: Smaller ``windowSize`` may help
3. **Adjust frequency bands**: Ensure bands cover the signal frequency content


References
==========

Primary Reference
-----------------

Lomax, A., Satriano, C., & Vassallo, M. (2012).
Automatic picker developments and optimization: FilterPicker - a robust,
broadband picker for real-time seismic monitoring and earthquake early-warning.
*Seismological Research Letters*, 83(3), 531-540.
https://doi.org/10.1785/gssrl.83.3.531

Additional References
---------------------

Vassallo, M., Satriano, C., & Lomax, A. (2012).
Automatic picker developments and optimization: A strategy for improving
the performances of automatic phase pickers.
*Seismological Research Letters*, 83(3), 541-554.

Original Implementation
-----------------------

The original FilterPicker implementation by A. Lomax is available at:

- http://alomax.free.fr/FilterPicker/
- http://alomax.net/projects/java/net/alomax/timedom/FilterPicker5.java


See Also
========

- :ref:`scautopick` - Automatic picking module
- :ref:`picker-gfz` - GFZ picker implementation
- :ref:`picker-aic` - AIC picker implementation
- :ref:`processing-picks` - Pick processing in SeisComP


Authors
=======

FilterPicker algorithm developed by Anthony Lomax, Claudio Satriano, and Maurizio Vassallo.

SeisComP implementation by Donavin Liebgott.


License
=======

This implementation is part of SeisComP and is licensed under the GNU Affero General Public License version 3.0 (AGPL-3.0).
