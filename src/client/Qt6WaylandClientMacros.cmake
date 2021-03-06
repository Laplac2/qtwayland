function(qt6_generate_wayland_protocol_client_sources target)
    qt_parse_all_arguments(arg "qt6_generate_wayland_protocol_client_sources" "" "" "FILES" ${ARGN})
    get_target_property(target_binary_dir ${target} BINARY_DIR)

    foreach(protocol_file IN LISTS arg_FILES)
        get_filename_component(protocol_name "${protocol_file}" NAME_WLE)

        set(waylandscanner_header_output "${target_binary_dir}/wayland-${protocol_name}-client-protocol.h")
        set(waylandscanner_code_output "${target_binary_dir}/wayland-${protocol_name}-protocol.c")
        # TODO: Maybe add "client" prefix or suffix to these in Qt6?
        set(qtwaylandscanner_header_output "${target_binary_dir}/qwayland-${protocol_name}.h")
        set(qtwaylandscanner_code_output "${target_binary_dir}/qwayland-${protocol_name}.cpp")

        add_custom_command(
            OUTPUT "${waylandscanner_header_output}"
            #TODO: Maybe put the files in ${CMAKE_CURRENT_BINARY_DIR/wayland_generated instead?
            COMMAND Wayland::Scanner --include-core-only client-header < "${protocol_file}" > "${waylandscanner_header_output}"
        )

        add_custom_command(
            OUTPUT "${waylandscanner_code_output}"
            COMMAND Wayland::Scanner --include-core-only code < "${protocol_file}" > "${waylandscanner_code_output}"
        )

        # TODO: Make this less hacky
        set(wayland_include_dir "")
        get_target_property(is_for_module "${target}" INTERFACE_MODULE_HAS_HEADERS)
        if (is_for_module)
            set(wayland_include_dir "QtWaylandClient/private")
        endif()

        add_custom_command(
            OUTPUT "${qtwaylandscanner_header_output}"
            COMMAND Qt6::qtwaylandscanner client-header "${protocol_file}" "${wayland_include_dir}" > "${qtwaylandscanner_header_output}"
        )

        # TODO: We need this hack in order to get the xcomposite plugins to build...
        # unfortunately, it's not going to work outside QtWayland because we're using waylandclient-private includes
        set(qtwaylandscanner_code_include "")
        set (targets_that_need_include
            "QWaylandXCompositeEglClientBufferPlugin"
            "QWaylandXCompositeGlxClientBufferPlugin"
            "QWaylandXCompositeEglPlatformIntegrationPlugin"
            "QWaylandXCompositeGlxPlatformIntegrationPlugin")
        if ("${target}" IN_LIST targets_that_need_include OR is_for_module)
            set(qtwaylandscanner_code_include "<QtWaylandClient/private/wayland-wayland-client-protocol.h>")
        endif()

        add_custom_command(
            OUTPUT "${qtwaylandscanner_code_output}"
            COMMAND Qt6::qtwaylandscanner client-code "${protocol_file}" --header-path='${wayland_include_dir}' --add-include='${qtwaylandscanner_code_include}' > "${qtwaylandscanner_code_output}"
        )

        target_sources(${target} PRIVATE
            "${waylandscanner_header_output}"
            "${waylandscanner_code_output}"
            "${qtwaylandscanner_header_output}"
            "${qtwaylandscanner_code_output}"
        )
    endforeach()
    target_include_directories(${target} PRIVATE ${target_binary_dir})
endfunction()

