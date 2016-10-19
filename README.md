# librouteros

**librouteros** is a library to communicate with *RouterOS*, the operating
system of MikroTik's RouterBoards. It uses the *API port* provided by those
systems to connect and talk to the devices. librouteros is a low-level library
in that it abstracts the network protocol used but has next to no knowledge
about the commands and responses available. Should such an high-level interface
prove useful, it will be added as the need arises.


## Dependencies

The RouterOS API uses a simple challenge-response authentication schema using
cryptographically secure MD5 hashes. The library used for MD5 computation is
*gcrypt*, a cryptographic library originally written for the GnuPG project.

## Enabling API connections

API connections (sometimes also called “Winbox connections”) must be explicitly
enabled. To do so, issue the following command:

    /ip service enable api

To allow a specific user / group to use the API, the “winbox” item must be
added to the user's “policy” in “/user group”.

## Contact

There's currently no mailing list available for librouteros. In case of
questions, suggestions and feedback in general, please contact the author
directly (see [Author](#Author) below).


## License

librouteros is licensed under the terms of the ISC License. Terms of the
license can be found in the file `COPYING`.


## See also

Information about MikroTik and RouterOS can be found on [MikroTik's
homepage](http://www.mikrotik.com/). The API documentation used to write this
library is available from [the Mikrotik
wiki](http://wiki.mikrotik.com/wiki/API).


## Author

*librouteros* was originally written in 2009 by *Florian Forster*
&lt;ff at octo.it&gt; (@octo on Github).

Additional contributors are listed in the file called `AUTHORS`.
