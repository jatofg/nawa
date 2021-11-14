if (NOT NAWA_FILES_CONFIGURED)
    set(NAWA_FILES_CONFIGURED ON)
    configure_file(${NAWA_SOURCE_DIR}/include/Application.h.in
            ${PROJECT_BINARY_DIR}/include/nawa/Application.h)
    configure_file(${NAWA_SOURCE_DIR}/internal/systemconfig.h.in
            ${PROJECT_BINARY_DIR}/include/nawa/systemconfig.h)
endif()
