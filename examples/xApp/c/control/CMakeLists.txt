if(XAPP_DB STREQUAL "MYSQL_XAPP")
	include_directories(${LIBMYSQLCLIENT_INCLUDE_DIRS})
endif()

# xapp_mac_ctrl
add_executable(xapp_mac_ctrl
								xapp_mac_ctrl.c
								${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
								)

target_link_libraries(xapp_mac_ctrl
											PUBLIC
											e42_xapp
											$<TARGET_OBJECTS:e2_time_obj>
											$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
											-pthread
											-lsctp
											-ldl
											)

# xapp_cust_slice_moni_ctrl
add_executable(xapp_cust_slice_moni_ctrl
		xapp_cust_slice_moni_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
)

target_link_libraries(xapp_cust_slice_moni_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
)

# xapp_oran_ctrl
add_executable(xapp_oran_slice_ctrl
        xapp_oran_slice_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_oran_slice_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# xapp_sense_ctrl
add_executable(xapp_sense_ctrl
        xapp_sense_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_sense_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# xapp_cust_sense_moni_ctrl
add_executable(xapp_cust_sense_moni_ctrl
        xapp_cust_sense_moni_ctrl.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_cust_sense_moni_ctrl
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# xapp_cust_sense_moni_ctrl_cmd
add_executable(xapp_cust_sense_moni_ctrl_cmd
        xapp_cust_sense_moni_ctrl_cmd.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_cust_sense_moni_ctrl_cmd
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)

# xapp_cust_sense_moni_ctrl_cmd
add_executable(xapp_cust_sense_moni_ctrl_cmd_slot
        xapp_cust_sense_moni_ctrl_cmd_slot.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_cust_sense_moni_ctrl_cmd_slot
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)


# xapp_cust_sense_moni_ctrl_cmd
add_executable(xapp_cust_sense_moni_ctrl_cmd_slot_target
        xapp_cust_sense_moni_ctrl_cmd_slot_target.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_cust_sense_moni_ctrl_cmd_slot_target
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)




# xapp_cust_sense_moni_ctrl_basic
add_executable(xapp_cust_sense_moni_ctrl_basic2
        xapp_cust_sense_moni_ctrl_basic2.c
		${CMAKE_SOURCE_DIR}/src/util/alg_ds/alg/defer.c
		)

target_link_libraries(xapp_cust_sense_moni_ctrl_basic2
		PUBLIC
		e42_xapp
		$<TARGET_OBJECTS:e2_time_obj>
		$<$<STREQUAL:${XAPP_DB},MYSQL_XAPP>:${LIBMYSQLCLIENT_LIBRARIES}>
		-pthread
		-lsctp
		-ldl
		)
