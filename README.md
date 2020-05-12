Ring buffer in C
===

This is an implementation of a ring buffer in C, with the following attributes:

- Ring buffer with user defined data type.
- Manages read/write and mutliple buffer instances.
- Buffer instances / slots can be claimed and returned for repeated use.
- The buffer struct is hidden to the user.
- Data has to be provided by the user.
- Memory usage and execution time not known yet, both expected to be very low.
  
Unit tests in Ceedling are planned.
