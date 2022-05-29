cmake_minimum_required(VERSION 3.14.0)

set(vst3sdk_SOURCE_DIR CACHE PATH ../vst3sdk/)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()
if(NOT IS_DIRECTORY ${vst3sdk_SOURCE_DIR})
    message(FATAL_ERROR "Path to VST3 SDK is not a directory!")
endif()

project(MVerb
    VERSION 1.0.0.0 
    DESCRIPTION "MVerb VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(MVerb
    source/vst3/version.h
    source/vst3/cids.h
    source/vst3/processor.h
    source/vst3/processor.cpp
    source/vst3/controller.h
    source/vst3/controller.cpp
    source/vst3/entry.cpp
    source/MVerb.h
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(MVerb
        PRIVATE
            resource/editor.uidesc
    )
    target_link_libraries(MVerb
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(MVerb
        RESOURCES
            "resource/editor.uidesc"
    )
endif(SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots (MVerb
    RESOURCES
        resource/B2D18CA401105C1AB7F76B14FEE77D9C_snapshot.png
        resource/B2D18CA401105C1AB7F76B14FEE77D9C_snapshot_2.0x.png
)

target_link_libraries(MVerb
    PRIVATE
        sdk
)

smtg_target_configure_version_file(MVerb)

if(SMTG_MAC)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)
    smtg_target_set_bundle(MVerb
        BUNDLE_IDENTIFIER com.martineastwood.MVerb.vst3
        COMPANY_NAME ""
    )
    smtg_target_set_debug_executable(MVerb
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(MVerb PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MVerb)

        smtg_target_set_debug_executable(MVerb
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
