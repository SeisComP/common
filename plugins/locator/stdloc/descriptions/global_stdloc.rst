StdLoc is a SeisComP locator plugin that combines standard location methods
and was developed with the focus on local seismicity, although the methods
are generic enough to work at different scales.

Plugin
======

To enable StdLoc the plugin ``stdloc`` must be loaded.


How does it work?
=================

The locator can apply a multitude of location methods and it is particularly useful to
combine them to achieve better solutions.
The methods available are: LeastSquares, GridSearch, OctTree and, more interestingly,
two mixed methods: GridSearch+LeastSquares and OctTree+LeastSquares:

- LeastSquares: this is the classic algorithm that solve the linearized problem via
  iterative Least Square. However an initial location estimate is required and that
  makes its solutions dependent on a good initial location. For this reason this
  method should be combined with GridSearch or OctTree, unless it is intended to be
  used only as a re-locator of an existing solution e.g. in scolv or screloc
- GridSearch: finds the source location by evaluating the hypocenter probability
  of each grid cell and returning the maximum likelihood hypocenter.
  The source time is derived from the weighted average of arrival travel times.
  The downside of this method is that the resolution of the hypocenters depends on
  the grid size, but dense grids can be very slow to compute,
- GridSearch+LeastSquares: this method works similarly to GridSearch but it performs
  an additional LeastSquares for each grid cell, using the cell centroid as initial
  location estimate. This solves the major issues of both LeastSquares and
  GridSearch: by trying multiple initial location estimates allow LeastSquares to
  not be dependent on a bad initial location estimate and the grid doesn't need to
  be dense, which makes the method faster than GridSearch and with higher resolution.
  For very local seisicity monitoring it could be used with a single cell only,
  which corresponds to starting LeastSquares from the same location with every
  event.
- OctTree: this method follows the NonLinLoc approach. The OctTree search starts 
  similarly to GridSearch by evaluating the hypocenter probability of each grid cell,
  computed as the probability density at the cell center coordinates times the cell
  volume. The search then continues by repeatedly fetching the cell with high 
  probability and dividing it in 8 sub-cells. These 8 cells are then inserted in the
  pool of cells to fetch from at next iteration.
  The search terminates after either a maximum number of iterations or after
  reaching a minimum cell size. At that point the maximum likelihood hypocenter
  is selected.
- OctTree+LeastSquares: the solution can be further improved combining OctTree with
  the Least Squares algorithm, which can use the OctTree solution as initial
  location estimate. This allows OctTree to stop after reaching a big cell size
  (i.e. it is fast) and letting LeastSquares to refine the solution. Knowing that
  the initial location estimate for LeastSquares is the maximum probability cell of
  OctTree the solution should not get stuck in a local minima. For example it is
  possible to define a grid size that covers a whole network and set the OctTree
  cell minimum size to a couple of kilometers. LeastSquares will then improve the
  location resolution of that coarse grid.


The algorithms implemented in StdLoc are standard methods described in "Routine Data
Processing in Earthquake Seismology" by Jens Havskov and Lars Ottemoller. The OctTree
search algorithm is based on NonLibLoc by Antony Lomax.



Why is stdloc suitable for local seismicity?
============================================

When dealing with very local seismicity (few kilometers or hundreds of meters) common
simplification for regional seismicity become important. In particular the locator
should take into consideration:
- station elevation and even negative elevation (e.g. borehole sensors)
- earthquake location can be above a seismic sensor (e.g. borehole sensors)
- possible negative earthquake depth (above surface)

More importantly the travel time tables used by the locator must be able to take
into consideration all the above too.

StdLoc is simple, but can deal with all this requirements.


Travel Time Table
=================

StdLoc works with any Travel Time Table type provided in SeisComP, however only
`homogeneous` takes into consideration station elevation and negative source depth,
which are important for very local seismicity.



