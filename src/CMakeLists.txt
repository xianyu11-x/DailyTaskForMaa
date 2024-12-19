add_subdirectory(mysqlConnectPool)
add_subdirectory(libtest)

set(SRC
    main.cpp
    handler/handler.cpp
    router/router.cpp
    mysqlConnectPool/sqlConnectPool.cpp
    sqlMap/sqlMap.cpp)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE co_async mysqlclient)