# Kraft Build and Installation

This document describes how to compile and install Kraft. This requires a bit of Linux knowledge and is only recommended for experienced users and developers.

## Packages

Linux Distributions have package management systems to provide users
with all kind of software in a clean and easy way.
This should be the prefered way to install Kraft. Check the package
pools of your prefered distribution first.

If the package is outdated, consider asking your distribution to upgrade!

## Compiling Kraft

The following section briefly describes how to build Kraft with cmake.

### Precondition

Check that cmake is installed on your machine and is in your PATH.
To do so, just type

```
$ cmake --version
```

on your command line. Version 2.4 is required, the most recent
stable version of cmake is preferred.

To build Kraft, the following libs and software versions have to
be provided:
- cmake and the cmake extra modules
- Qt libs incl. devel packages version 5.5.0 or later
- kcontacts for using the KDE contact classes
- A few other KDE classes (kxmlgui, ki18n)
- google ctemplate, A simple but powerful template language for C++,
  packages from the openSUSE Buildservice or from the website
  https://github.com/OlafvdSpek/ctemplate
- grantlee, an C++ text template framework
- optional: akonadi contact for Akonadi based addressbook access

Required packages (names not accurate) for building with openSUSE:

- cmake
- extra-cmake-modules
- gcc-c++
- kcontacts-devel
- gettext
- libctemplate-devel
- libQt5Core-devel
- libQt5Gui-devel
- libQt5Sql-devel
- libQt5Widgets-devel
- libQt5Xml-devel
- libQt5Svg-devel
- libQt5XmlPatterns
- grantlee5-devel
- kf5-i18n
- kf5-Config
- kf5-Contacts
- grantlee5-devel

These are optional to build with Akonadi Support:
- akonadi-contact-devel
- akonadi-devel
- kf5-ContactEditor

To build with Akonadi versions before 23.04, cmake has to run wtih the
build option `-DAKONADI_LEGACY_BUILD=ON` to use the old prefix KF5.

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

To start Kraft from the build directory, set the environment variable
`KRAFT_HOME` to the root of the _source_ directory to let Kraft find its
resource files:
```
$ KRAFT_HOME=/home/me/sources/kraft
```

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
server with appropiate permissions to write to a specific
database and create tables on it. Create an empty database
to use with Kraft. Remember both the database name and the
credentials.
On Krafts first start, enter these data in the setup assistant.
Kraft will create the database tables and fill it automatically.

## Document Generation

Kraft generates PDF documents. For that it uses either a python tool named
erml2pdf or the python project weasyprint. erml2pdf can be found in Kraft's tools directory in this source package. Weasyprint should be installed separately on the machine that is running Kraft.

To compute PDF watermarks, Kraft uses python-pypdf2 for pdf processing. The python modules are not part of Kraft and should be installed separately on the system.



