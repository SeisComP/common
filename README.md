# SeisComP common repository

This repository includes the common SeisComP framework C++ libraries
and Python wrappers. It does not contain applications that implement
a particular workflow, e.g. signal processing or graphical user
interfaces for data analysis. Those are part of the `main` repository.

This repository cannot be built standalone. It needs to be integrated
into the `seiscomp` build environment and checked out into
`src/base/common`.

```
$ git clone [host]/seiscomp.git
$ cd seiscomp/src/base
$ git clone [host]/common.git
```

# Build

## Configuration

|Option|Default|Description|
|------|-------|-----------|
|SC_TRUNK_DB_MYSQL|ON|Enable MySQL database support.|
|SC_TRUNK_DB_POSTGRESQL|OFF|Enable PostgreSQL database support.|
|SC_TRUNK_DB_SQLITE3|OFF|Enable SQLite3 database support.|

## Compilation

Follow the build instructions from the `seiscomp` repository.
