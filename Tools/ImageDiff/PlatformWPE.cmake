set(IMAGE_DIFF_SOURCES
    ${IMAGE_DIFF_DIR}/cairo/ImageDiff.cpp
)

list(APPEND IMAGE_DIFF_SYSTEM_INCLUDE_DIRECTORIES
    ${CAIRO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

list(APPEND IMAGE_DIFF_LIBRARIES
    ${CAIRO_LIBRARIES}
    ${GLIB_LIBRARIES}
)
