find_package(fmt REQUIRED)
if (NOT TARGET fmt::fmt)
  message(FATAL_ERROR "Please install fmtlib (a.k.a., {fmt})")
endif ()
