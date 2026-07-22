# Build targets for spec sidecar tooling in ./tools/specdoc

find_package(Uv REQUIRED)

set(SPEC_SIDECAR ${CMAKE_SOURCE_DIR}/data/dev.ulduar.Constellar1.spec.yaml)
set(SPEC_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
set(ACTIVITY_KEYS_HEADER ${CMAKE_SOURCE_DIR}/src/contract/ActivityKeys.h)
set(SPECDOC_RUN ${UV_EXECUTABLE} run --project
                ${CMAKE_SOURCE_DIR}/tools/specdoc specdoc)

add_custom_target(
    spec-gen
    COMMAND ${SPECDOC_RUN} --emit keys --out ${ACTIVITY_KEYS_HEADER}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Regenerating ActivityKeys.h from the spec sidecar"
    VERBATIM)

add_custom_target(
    check-spec
    COMMAND ${SPECDOC_RUN} --emit keys --out ${ACTIVITY_KEYS_HEADER} --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking ActivityKeys.h against the spec sidecar"
    VERBATIM)

add_custom_target(
    spec-doc
    COMMAND ${SPECDOC_RUN} --emit html --license ${SPEC_LICENSE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Rendering the D-Bus spec reference to doc/spec/"
    VERBATIM)
