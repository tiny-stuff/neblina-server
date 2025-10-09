foreach (TARGET ${SPECIAL_TARGETS})

    add_custom_target(
            ${TARGET}-leaks
            COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=${CMAKE_SOURCE_DIR}/valgrind.supp ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking leaks on ${TARGET}...
            VERBATIM
    )

    add_custom_target(
            ${TARGET}-helgrind
            COMMAND valgrind --tool=helgrind ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking threads on ${TARGET} with helgrind...
            VERBATIM
    )

    add_custom_target(
            ${TARGET}-drd
            COMMAND valgrind --tool=drd ./${TARGET}
            DEPENDS ${TARGET}
            COMMENT Checking threads on ${TARGET} with drd...
            VERBATIM
    )

endforeach ()