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
find_path(MYSQL_INCLUDE_DIR NAMES mysql.h PATH_SUFFIXES mysql mariadb)
find_library(MYSQL_CLIENT_LIBRARY NAMES mysqlclient mariadbclient mariadb)

if (NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_CLIENT_LIBRARY)
    message(FATAL_ERROR "MySQL/MariaDB client headers or library not found")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${MYSQL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE co_async ${MYSQL_CLIENT_LIBRARY})
