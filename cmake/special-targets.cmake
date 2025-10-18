foreach (TARGET ${FINAL_TARGETS})

    add_custom_target(
            ${TARGET}-leaks
            COMMAND ${CMAKE_COMMAND} -E env VALGRIND=1 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --suppressions=${CMAKE_SOURCE_DIR}/valgrind.supp ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking leaks on ${TARGET}...
            VERBATIM
    )

    add_custom_target(
            ${TARGET}-helgrind
            COMMAND ${CMAKE_COMMAND} -E env VALGRIND=1 valgrind --tool=helgrind ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking threads on ${TARGET} with helgrind...
            VERBATIM
    )

    add_custom_target(
            ${TARGET}-drd
            COMMAND ${CMAKE_COMMAND} -E env VALGRIND=1 valgrind --tool=drd ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking threads on ${TARGET} with drd...
            VERBATIM
    )

endforeach ()