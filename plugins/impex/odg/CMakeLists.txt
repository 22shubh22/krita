set(kritaodgimport_SOURCES
    kis_odg_import.cc
)

add_library(kritaodgimport MODULE ${kritaodgimport_SOURCES})

include_directories(SYSTEM ${PNG_INCLUDE_DIR})

add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

target_link_libraries(kritaodgimport kritaui ${PNG_LIBRARIES} )

install(TARGETS kritaodgimport  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

install( PROGRAMS  krita_odg.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
