add_executable(logtail main.cpp)
target_link_libraries(logtail PRIVATE Qt6::Core wowcapd-log wowcapd-controller)
