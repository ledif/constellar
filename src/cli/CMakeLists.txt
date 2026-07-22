add_executable(constellarctl main.cpp)

target_link_libraries(constellarctl
  PRIVATE
    Qt6::Core
    Qt6::DBus
    constellar::client
    constellar::common
    constellar::contract
)

install(TARGETS constellarctl)
