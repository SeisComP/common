The waveform archive server is a small application that serves a local
SDS archive via different protocols. Currently there are two implementations:

* :ref:`fdsnws dataselect <sec-dataSelect>`

  * dataselect/1/query
  * dataselect/1/version
  * dataselect/1/application.wadl

All data are forwarded unrestricted. There are no options to add restriction
checks or user authentication.
