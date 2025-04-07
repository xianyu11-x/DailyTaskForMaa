add_subdirectory(mysqlConnectPool)
#add_subdirectory(libtest)
add_subdirectory(levelManager)

set(SRC
    main.cpp
    handler/getStrategy.cpp
    handler/getTask.cpp
    handler/quickTask.cpp
    handler/reportStatus.cpp
    handler/updateLevel.cpp
    handler/updateStrategy.cpp
    router/router.cpp
    mysqlConnectPool/sqlConnectPool.cpp
    sqlMap/sqlMap.cpp
    levelManager/levelManager.cpp
    conf/conf.cpp
    )

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE co_async mysqlclient)