function(add_make_dist_target _name _version)
    set(ARCHIVE_NAME ${_name}-${_version})
    add_custom_target(dist
        COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
            | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endfunction(add_make_dist_target)
