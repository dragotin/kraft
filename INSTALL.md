# Build Kraft from Source

This document describes how to compile and install Kraft. This requires a bit of Linux knowledge and is only recommended for experienced users and developers.

### Precondition

Check that cmake is installed on your machine and is in your PATH.
To do so, just type

```
$ cmake --version
```

on your command line. Version 3.16 is required, the most recent
stable version of cmake is preferred.

To build Kraft, the following libs and software versions have to
be provided:
- cmake and the cmake extra modules
- Qt libs incl. devel packages version 6.2.4 or later
- kcontacts for using the KDE contact classes
- A few other KDE Frameworks 6 classes (ktexttemplate, kxmlgui, ki18n)
- optional: akonadi contact for Akonadi based addressbook access

### Build Kraft

cmake is designed so that the build process can be done in a separate
directory. This is highly recommended for users and required for packagers.

Go to the top level of the source directory.
To build Kraft in the subdirectory `./build/` type

```
$ mkdir build
$ cd build
$ cmake ..
    # to generate the Makefiles.
$ cmake .
    # to change the configuration of the build process. (optional)
```

Check out for errors during the cmake run. Fix them, usually you need
more devel packages installed.

Ready? Congratulations, your Makefiles were generated!
Now you could just type

```
$ make
    # to build the project in the build/ directory.
```

Note that 'make' automatically checks whether any CMakeLists.txt file
has changed and reruns cmake if necessary.

### Run Kraft from Buildenvironment

To start Kraft from the build directory, set the environment variable
`KRAFT_HOME` to the root of the _source_ directory to let Kraft find its
resource files:
```
$ export KRAFT_HOME=/home/me/sources/kraft
```
That is useful for running Kraft from an IDE.

## Kraft Installation

Type
```
$ make install
```

To change the target root directory to where it is installed, call
cmake with the parameter `-DCMAKE_INSTALL_PREFIX=/my_install_dir`

## Kraft Manual

Kraft ships a user manual in different languages. To rebuild it, asciidoctor
is required. If that dependency is found, cmake detects it and gives a new
make target:

```
$ make manual
```

to re-create the docs.

## Database

Kraft either can use a SQLite file based database or a MySQL server based
database.

The SQLite database is created automatically on the fly
on first start. Its use is recommended for all users who
want to evaluate Kraft.

To run Kraft with MySQL, create or pick a user on the MySQL
server with appropriate permissions to write to a specific
database and create tables on it. Create an empty database
to use with Kraft. Remember both the database name and the
credentials.
On Krafts first start, enter these data in the setup assistant.
Kraft will create the database tables and fill it automatically.

## Document Generation

Kraft generates PDF documents. For that it uses the python project weasyprint. 
Weasyprint should be installed separately on the machine that is running Kraft.

To compute PDF watermarks, Kraft uses python-pypdf2 for pdf processing. The python 
modules are not part of Kraft and need to be installed separately on the system.



