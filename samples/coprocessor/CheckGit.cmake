
set(CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})
if (NOT DEFINED coprocessor_pre_configure_dir)
    set(coprocessor_pre_configure_dir ${CMAKE_CURRENT_LIST_DIR})
endif ()

if (NOT DEFINED coprocessor_post_configure_dir)
    set(coprocessor_post_configure_dir ${CMAKE_BINARY_DIR}/generated)
endif ()

set(coprocessor_pre_configure_file ${coprocessor_pre_configure_dir}/src/rnb_coprocessor_version.c.in)
set(coprocessor_post_configure_file ${coprocessor_post_configure_dir}/rnb_coprocessor_version.c)

function(CheckCoprocessorGitWrite git_hash)
    file(WRITE ${CMAKE_BINARY_DIR}/git-state.txt ${git_hash})
endfunction()

function(CheckCoprocessorGitRead git_hash)
    if (EXISTS ${CMAKE_BINARY_DIR}/git-state.txt)
        file(STRINGS ${CMAKE_BINARY_DIR}/git-state.txt CONTENT)
        LIST(GET CONTENT 0 var)

        set(${git_hash} ${var} PARENT_SCOPE)
    endif ()
endfunction()

function(CheckCoprocessorGitVersion)
    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE COPROCESSOR_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )

    message(STATUS "COPROCESSOR_GIT_HASH ${COPROCESSOR_GIT_HASH}")

    CheckCoprocessorGitRead(GIT_TAG_CACHE)
    if (NOT EXISTS ${coprocessor_post_configure_dir})
        file(MAKE_DIRECTORY ${coprocessor_post_configure_dir})
    endif ()

    if (NOT EXISTS ${coprocessor_post_configure_dir}/rnb_coprocessor_version.h)
        file(COPY ${coprocessor_pre_configure_dir}/src/rnb_coprocessor_version.h DESTINATION ${coprocessor_post_configure_dir})
    endif()

    if (NOT DEFINED GIT_TAG_CACHE)
        set(GIT_TAG_CACHE "INVALID")
    endif ()

    # Only update the rnb_coprocessor_version.c if the hash has changed. This will
    # prevent us from rebuilding the project more than we need to.
    if (NOT "${COPROCESSOR_GIT_HASH}" STREQUAL "${GIT_TAG_CACHE}" OR NOT EXISTS ${coprocessor_post_configure_file})
        # Set che GIT_TAG_CACHE variable the next build won't have
        # to regenerate the source file.
        CheckCoprocessorGitWrite(${COPROCESSOR_GIT_HASH})

        configure_file(${coprocessor_pre_configure_file} ${coprocessor_post_configure_file} @ONLY)
    endif ()

endfunction()

function(CheckCoprocessorGitSetup)

    add_custom_target(AlwaysCheckCoprocessorGit COMMAND ${CMAKE_COMMAND}
        -DRUN_CHECK_GIT_VERSION=1
        -Dcoprocessor_pre_configure_dir=${coprocessor_pre_configure_dir}
        -Dcoprocessor_post_configure_file=${coprocessor_post_configure_dir}
        -DGIT_TAG_CACHE=${GIT_TAG_CACHE}
        -P ${CURRENT_LIST_DIR}/CheckGit.cmake
        BYPRODUCTS ${coprocessor_post_configure_file}
        )

    CheckCoprocessorGitVersion()
endfunction()

# This is used to run this function from an external cmake process.
if (RUN_CHECK_GIT_VERSION)
    CheckCoprocessorGitVersion()
endif ()
