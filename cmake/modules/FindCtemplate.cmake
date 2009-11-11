# - Try to find the ctemplate
# Once done this will define
#
#  CTEMPLATE_FOUND - system has libspectre
#  CTEMPLATE_INCLUDE_DIR - the libspectre include directory
#  CTEMPLATE_LIBRARIES - Link this to use libspectre
#

# Copyright (c) 2009, Thomas Richard, <thomas.richard@proan.be>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)
  # in cache already
  SET(CTEMPLATE_FOUND TRUE)

else (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)
  FIND_PATH(CTEMPLATE_INCLUDE_DIR ctemplate/template.h)
  FIND_LIBRARY(CTEMPLATE_LIBRARIES NAMES ctemplate)

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Ctemplate DEFAULT_MSG CTEMPLATE_INCLUDE_DIR CTEMPLATE_LIBRARIES )

endif (CTEMPLATE_INCLUDE_DIR AND CTEMPLATE_LIBRARIES)