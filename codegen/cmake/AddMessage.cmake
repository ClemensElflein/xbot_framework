function(add_message MESSAGE_NAME MESSAGE_FILE)
    add_custom_command(
            OUTPUT
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/${MESSAGE_NAME}_encode.c
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/${MESSAGE_NAME}_decode.c
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/include/${MESSAGE_NAME}_encode.h
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/include/${MESSAGE_NAME}_decode.h
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/include/${MESSAGE_NAME}_types.h
            COMMAND
            ${ZCBOR_CODEGEN_EXECUTABLE} code
            -c ${MESSAGE_FILE}
            --encode --decode
            --short-names
            -t ${MESSAGE_NAME}
            --output-c ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/${MESSAGE_NAME}.c
            --output-h ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/include/${MESSAGE_NAME}.h
            DEPENDS
            ${MESSAGE_FILE}
    )

    add_library(ZCBOR_${MESSAGE_NAME} OBJECT EXCLUDE_FROM_ALL)
    target_sources(ZCBOR_${MESSAGE_NAME} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/${MESSAGE_NAME}_encode.c
            ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/${MESSAGE_NAME}_decode.c
    )
    target_include_directories(ZCBOR_${MESSAGE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated/zcbor/${MESSAGE_NAME}/include)
    target_link_libraries(ZCBOR_${MESSAGE_NAME} PUBLIC zcbor)
endfunction()
