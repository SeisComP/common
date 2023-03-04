StdLoc is a SeisComP locator plugin with focus on local seismicity.

Plugin
======

To enable StdLoc the plugin ``stdloc`` must be loaded.


How does it work?
=================

The locator can find a solution via iterative least squares, grid search or, more
interestingly, as a mix of the two. In this latter case a coarse grid should be
selected and an iterative least square solution is found for each cell. The solution
with lowest error is preferred and returned. This mixed approach is interesting since
it solves the issue of the least square method that requires an initial location
estimate.

A special case of this configuration is to have a grid with only one cell so that the
least square will always start from that cell centroid. That configuration would make
sense when,for example, monitoring working sites and the initial location is actually
known.

The algorithms implemented in StdLoc are standard methods described in 
"Routine Data Processing in Earthquake Seismology" by Jens Havskov and
Lars Ottemoller


Why is it for local seismicity?
===============================

When dealing with very local seismicity (few kilometers or hundres of meters) common
semplification for regional seismicity become importants. In particular the locator
should take into consideration:
- station elevation and even negative elevation (e.g. borehole sensors)
- earthquake location can be above a seismic sensor (e.g. borehole sensors)
- possible negative earthquake depth (above surface)

More importantly the travel time tables used by the locator must be able to take
into consideration all the above too.

GenLon is simple, but can deal with all this requirements. It also supports any SeisComP
travel time table, which means it can also be configured with the `homogeneous` model,
which is able to take into consideration the above points.


Travel Time Table
=================

StdLoc works with any Travel Time Table type provided in SeisComP, however only
`homogeneous` takes into consideration station elevation and negative source depth,
which are important for very local seismicity.



