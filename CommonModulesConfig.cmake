# - Config file for the CommonModules package
# It adds CommonModules to the module path

# Compute paths
get_filename_component(COMMONMODULES_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(CMAKE_MODULE_PATH ${COMMONMODULES_CMAKE_DIR} ${CMAKE_MODULE_PATH})
