---
title: librouteros
layout: post
---
**librouteros** (or *libRouterOS*) is a *C&nbsp;library* to communicate with network devices by *MikroTik* running their
Linux-based operating system *RouterOS*. It currently is a very low-level library in that is only abstracts the records
based network protocol, not the commands, arguments and filters provided via this interface.

The goal is to write a plugin for [collectd](https://collectd.org/) based on this library. Once that's done and
released, the interface of this library will be declared stable and version&nbsp;1.0 will be released. Until then, this
is a work in progress.

While it's hopefully easy to port the library to other operating systems, it has currently only be tested with
GNU&nbsp;/ Linux. Success stories and patches are very welcome, of course.

## News

*   **2010-03-29** Version **1.1.2** is available. Two memory leaks have been fixed in `ros_query` and `ros_connect`.
    Especially `ros_query` is subject to being run repeatedly and may cause an annoying growth of memory in an
    application.
*   **2010-01-04** Version **1.1.1** is available. The `ros_system_resource` command now returns memory and disk space
    in bytes, rather than “kilobytes” (actually [kibibytes](http://en.wikipedia.org/wiki/Binary_prefix)). The CPU
    frequency is also returned in *hertz*&nbsp;(Hz) rather than megahertz&nbsp;(MHz).
*   **2009-12-25** Version **1.1.0** has been released. It adds a high-level interface for `/system/resource/print` to
    both, the library and the `ros` example application.
*   **2009-12-22** Version **1.0.1** has been released. It fixes potential segmentation faults when `ros_query` was
    called with `NULL` pointers. Also, compiling with g++, the GNU C++ compiler, has been simplified. Thanks to *Andrey
    Ponomarenko* for his feedback!
*   **2009-12-18** Version **1.0.0** has been released. It fixes some compiler warnings with GCC. Mostly, this release
    is there to show that the API is now considered stable.
*   **2009-11-27** Version **0.2.0** is available. High-level interfaces for `/interface/print` and
    `/interface/wireless/registration-table/print` have been added. The simple command line client `ros` can now be used
    to use the library from the command line, albeit not yet very useful.
*   **2009-11-25** The initial version, **0.1.0**, has been released.

## Download

*   [librouteros-1.1.2.tar.bz2](files/librouteros-1.1.2.tar.bz2) (source tarball)
*   [librouteros-1.1.2.tar.gz](files/librouteros-1.1.2.tar.gz) (source tarball)

### Development version

The development files are kept in a [Git](http://git-scm.com/) repository. You can “clone” it with the following
command. Patches are welcome anytime.&nbsp;:)

    git clone git://github.com/octo/librouteros.git

## Documentation

The only documentation right now is the manpage. You can find an online version here:

*   [librouteros(3)](manpages/librouteros.3.html)
*   [ros(1)](manpages/ros.1.html)

## See also

*   [mikrotik.com](http://www.mikrotik.com/), MikroTik's homepage.
*   [API description](http://wiki.mikrotik.com/wiki/API) in MikroTik's Wiki, including alternative libraries in other
    languages.

## License information

librouteros is distributed under the *ISC License*. The license can also be found in the `COPYING` file in the
repository.
