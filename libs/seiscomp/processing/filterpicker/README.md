# FilterPicker for SeisComP

A C++ implementation of the FilterPicker algorithm for SeisComP, based on the work of Lomax et al. (2012). Adapted for SeisComP by Donavin Liebgott.

## Overview

FilterPicker is a general-purpose, broadband phase detector and picker algorithm applicable to real-time seismic monitoring and earthquake early-warning systems. It operates on multiple frequency bands and uses characteristic functions to detect and pick seismic phases.

## Features

- **Broadband detection**: Operates on multiple frequency bands simultaneously
- **Robust picking**: Uses characteristic function integration for reliable onset detection
- **Adaptive thresholding**: Optional noise-adaptive threshold scaling
- **Uncertainty estimation**: Provides realistic timing uncertainty estimates
- **Polarity detection**: Determines onset polarity (positive/negative)
- **Reusable InPlaceFilter**: The characteristic function is available as a standalone `InPlaceFilter` for use with other pickers or preprocessing

## Algorithm

The FilterPicker algorithm works as follows:

1. **Filter Bank**: Input data is filtered through a bank of bandpass filters with logarithmically spaced center frequencies
2. **Characteristic Function**: For each frequency band, a characteristic function (CF) is computed based on the envelope STA/LTA ratio
3. **Integration**: The maximum CF across all bands is integrated over a time window
4. **Detection**: A pick is declared when the integral exceeds a threshold
5. **Refinement**: The exact onset time and uncertainty are estimated from the CF shape

## Components

### 1. FilterPicker Picker Plugin

The main picker plugin that implements the full FilterPicker algorithm. Use this in `scautopick` for automatic phase picking.

### 2. FilterPickerCF InPlaceFilter

A reusable `InPlaceFilter` that computes the FilterPicker characteristic function. This can be used:
- As a preprocessing filter before other pickers
- In custom processing chains
- For research and analysis of the CF behavior

Example usage of the InPlaceFilter:

```cpp
#include <seiscomp/math/filter/filterpicker.h>

// Create the CF filter
Math::Filtering::FilterPickerCF<double> cfFilter(
    1.0, 10.0,   // Low and high frequency (Hz)
    0.5, 10.0,   // STA and LTA windows (seconds)
    100.0        // Sampling frequency (Hz)
);

// Apply to data (in-place modification)
cfFilter.apply(numSamples, data);
// Now 'data' contains the characteristic function values
```

Or in Python:

```python
from seiscomp.math import Filtering

# Create the CF filter
cfFilter = Filtering.FilterPickerCFD(1.0, 10.0, 0.5, 10.0, 100.0)

# Apply to data
cfFilter.apply(data)
```

## Using FilterPickerCF as Preprocessing Filter

The `FilterPickerCF` InPlaceFilter can be used to preprocess data before applying other pickers. This is useful because:

1. **Enhanced phase onsets**: The CF emphasizes phase arrivals, making them easier to detect
2. **Noise robustness**: The CF is more robust to noise than raw amplitude
3. **Compatibility**: Works with any picker that accepts prefiltered data

### Example: Using CF with STA/LTA picker

```ini
# In scautopick.cfg

# First apply FilterPickerCF as preprocessing
filter = FilterPickerCF(1.0, 10.0, 0.5, 10.0)

# Then use standard STA/LTA picker on the CF-enhanced data
picker = STA/LTA
picker.STALTA.lenSTA = 0.5
picker.STALTA.lenLTA = 10.0
picker.STALTA.threshold = 3.0
```

### Programmatic usage in C++

```cpp
#include <seiscomp/math/filter/filterpicker.h>
#include <seiscomp/math/filter/stalta.h>

// Create the CF filter
Math::Filtering::FilterPickerCF<double> cfFilter(1.0, 10.0, 0.5, 10.0, fsamp);

// Create STA/LTA picker
Math::Filtering::STALTA<double> staltaPicker(0.5, 10.0, fsamp);

// Process data: first CF, then STA/LTA
cfFilter.apply(n, data);      // data now contains CF
staltaPicker.apply(n, data);  // data now contains CF-based STA/LTA
```

This approach allows you to combine the robustness of FilterPicker's characteristic function with the simplicity and speed of other picking algorithms.

## Installation

### Prerequisites

- SeisComP source code (scsrc for reference)
- CMake >= 3.10
- C++ compiler with C++11 support

### Build Instructions

1. The FilterPicker module is located in:
   ```
   scsrc/src/base/common/libs/seiscomp/processing/filterpicker/
   ```

2. The module is automatically included in the build when you compile SeisComP:
   ```bash
   cd scsrc/build
   cmake ..
   make -j4
   make install
   ```

3. Verify the plugin is available:
   ```bash
   ls -x ~/seiscomp/share/plugins/filterpicker.so
   ```

## Configuration

### Basic Configuration

To enable FilterPicker in scautopick, add to your `scautopick.cfg` or `global.cfg`:

```ini
# Enable FilterPicker for P-phase picking
picker = FilterPicker
```

### Parameters

#### FilterPicker (P-phase)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `picker.FilterPicker.numBands` | 5 | Number of frequency bands |
| `picker.FilterPicker.minFreq` | 0.5 | Minimum frequency (Hz) |
| `picker.FilterPicker.maxFreq` | 20.0 | Maximum frequency (Hz) |
| `picker.FilterPicker.windowSize` | 2 | Integration window (seconds) |
| `picker.FilterPicker.threshold` | 3.0 | Detection threshold |
| `picker.FilterPicker.thresholdOff` | 1.5 | Reset threshold |
| `picker.FilterPicker.adaptiveThreshold` | false | Enable adaptive thresholding |
| `picker.FilterPicker.noiseBegin` | -10.0 | Noise window start (seconds) |
| `picker.FilterPicker.signalBegin` | -5.0 | Signal window start (seconds) |
| `picker.FilterPicker.signalEnd` | 20.0 | Signal window end (seconds) |
| `picker.FilterPicker.minSNR` | 3.0 | Minimum SNR for valid pick |

### Example Configuration

See `filterpicker.cfg` for a complete example configuration file.

## Usage

### Real-time Processing

```bash
seiscomp exec scautopick --config-filter="picker=FilterPicker"
```

### Offline Processing

```bash
seiscomp exec scautopick --offline --playback -I data.mseed --ep > picks.xml
```

## Tuning Guidelines

### For Local/Regional Events

- Increase `maxFreq` to 30-50 Hz
- Use more frequency bands (6-8)
- Lower the threshold (2.0-2.5)

### For Teleseismic Events

- Decrease `minFreq` to 0.5-0.7 Hz
- Use fewer frequency bands (3-4)
- Increase windowSize to 3-5 seconds

### For Noisy Environments

- Enable adaptive thresholding
- Increase the threshold
- Increase minSNR

## Performance

The FilterPicker is designed for real-time operation. Performance depends on:

- Number of frequency bands (more bands = more CPU)
- Sampling rate (higher rate = more CPU)
- Window size (larger window = more latency)

Typical performance on modern hardware:
- 100 Hz data, 5 bands: < 10 ms per trace per second
- Memory usage: ~1 MB per active trace

## References

1. Lomax, A., Satriano, C., & Vassallo, M. (2012). Automatic picker developments and optimization: FilterPicker - a robust, broadband picker for real-time seismic monitoring and earthquake early-warning. *Seismological Research Letters*, 83(3), 531-540. https://doi.org/10.1785/gssrl.83.3.531

2. Vassallo, M., Satriano, C., & Lomax, A. (2012). Automatic picker developments and optimization: A strategy for improving the performances of automatic phase pickers. *Seismological Research Letters*, 83(3), 541-554.

## Original Implementation

The original FilterPicker implementation by A. Lomax is available at:
- http://alomax.free.fr/FilterPicker/
- http://alomax.net/projects/java/net/alomax/timedom/FilterPicker5.java

## License

This implementation is part of SeisComP and is licensed under the GNU Affero General Public License version 3.0 (AGPL-3.0).

## Author

Implementation based on the FilterPicker algorithm by Anthony Lomax, adapted for SeisComP by Donavin Liebgott.

## Support

For issues and questions, please refer to the SeisComP documentation or community forums.
