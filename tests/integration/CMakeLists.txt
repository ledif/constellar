add_test(NAME integration_status
    COMMAND dbus-run-session -- bash ${CMAKE_CURRENT_SOURCE_DIR}/smoke.sh
            $<TARGET_FILE:constellard> $<TARGET_FILE:constellarctl>
)

set_tests_properties(integration_status PROPERTIES
     LABELS integration
     TIMEOUT 30
)

add_test(NAME integration_contract
    COMMAND dbus-run-session -- python3 ${CMAKE_CURRENT_SOURCE_DIR}/contract.py
            $<TARGET_FILE:constellard>
)

set_tests_properties(integration_contract PROPERTIES
     LABELS integration
     TIMEOUT 30
)
