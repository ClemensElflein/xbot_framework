function(add_service SERVICE_NAME JSON_FILE)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/include/${SERVICE_NAME}Base.hpp ${CMAKE_CURRENT_BINARY_DIR}/generated/${SERVICE_NAME}Base.cpp
            COMMAND ${Python3_EXECUTABLE} -m cogapp -d -I ${XBOT_CODEGEN_PATH}/xbot_codegen -D service_file=${JSON_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/generated/include/${SERVICE_NAME}Base.hpp ${XBOT_CODEGEN_PATH}/templates/ServiceTemplate.hpp
            COMMAND ${Python3_EXECUTABLE} -m cogapp -d -I ${XBOT_CODEGEN_PATH}/xbot_codegen -D service_file=${JSON_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/generated/${SERVICE_NAME}Base.cpp ${XBOT_CODEGEN_PATH}/templates/ServiceTemplate.cpp
            DEPENDS ${XBOT_CODEGEN_PATH}/templates/ServiceTemplate.hpp ${XBOT_CODEGEN_PATH}/templates/ServiceTemplate.cpp ${JSON_FILE}
            COMMENT "Generating code for service ${SERVICE_NAME}."
    )

    add_library(${SERVICE_NAME} OBJECT EXCLUDE_FROM_ALL
    )

    target_sources(${SERVICE_NAME} PRIVATE             ${CMAKE_CURRENT_BINARY_DIR}/generated/${SERVICE_NAME}Base.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/generated/include/${SERVICE_NAME}Base.hpp
    )

    target_include_directories(${SERVICE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
    target_link_libraries(${SERVICE_NAME} PUBLIC xbot_comms)
endfunction()
