# ABSTRACT
This system creates and classifies images from an aerial reconassisance vehicle.  It is a series of small connected programs that implement a pipeline image processing system.

# DEPENDENCIES
- boost 1.48.0
- zeromq 2.1.10
- opencv 2.3.1

# Programs
## `bench.cc`
`bench` is an image simulation program to test various protocols, response sizes, and camera control conditions without the need of the uEye cameras. See `bench --help` for more details.

## `pipeclock.py`
`pipeclock.py` acts as a client and times how long responses take as well as provide any debugging detail of the response itself. This can be used in conjunction with `bench`. Both programs will look at the current user's `env` for the following:

- `PORT`: The port to run the given service on.
