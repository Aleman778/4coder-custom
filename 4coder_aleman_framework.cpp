
function void
aleman_4coder_initialize(Application_Links *app,
                     String_Const_u8_Array file_names,
                     i32 override_font_size,
                     b32 override_hinting) {
    Scratch_Block scratch(app);
    
    load_config_and_apply(app, &global_config_arena, override_font_size, override_hinting);

    // TODO(alexander): add support for file defined bindings later
    // String_Const_u8 bindings_file_name = string_u8_litexpr("bindings.4coder");
    // String_Const_u8 mapping = def_get_config_string(scratch, vars_save_string_lit("mapping"));
    
    // if (string_match(mapping, string_u8_litexpr("mac-default"))){
    //     bindings_file_name = string_u8_litexpr("mac-bindings.4coder");
    // } else if (OS_MAC && string_match(mapping, string_u8_litexpr("choose"))){
    //     bindings_file_name = string_u8_litexpr("mac-bindings.4coder");
    // }
    
    // if (dynamic_binding_load_from_file(app, &framework_mapping, bindings_file_name)){
        // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
    // } else {
        // setup_built_in_mapping(app, mapping, &framework_mapping, global_map_id, file_map_id, code_map_id);
        // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
    // }
    
    // open command line files
    String_Const_u8 hot_directory = push_hot_directory(app, scratch);
    for (i32 i = 0; i < file_names.count; i += 1){
        Temp_Memory_Block temp(scratch);
        String_Const_u8 input_name = file_names.vals[i];
        String_Const_u8 full_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                    string_expand(hot_directory),
                                                    string_expand(input_name));
        Buffer_ID new_buffer = create_buffer(app, full_name, BufferCreate_NeverNew|BufferCreate_MustAttachToFile);
        if (new_buffer == 0){
            create_buffer(app, input_name, 0);
        }
    }
}

CUSTOM_COMMAND_SIG(custom_startup) {
    ProfileScope(app, "custom startup");
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_Startup)){
        String_Const_u8_Array file_names = input.event.core.file_names;
        load_themes_default_folder(app);
        Face_Description description = get_face_description(app, 0);
        aleman_4coder_initialize(app, file_names,
                             description.parameters.pt_size,
                             description.parameters.hinting);
        default_4coder_side_by_side_panels(app, file_names);
        b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));

        if (auto_load) {
            Scratch_Block scratch(app);
            String_Const_u8 directory = def_get_config_string(scratch,
                                                              vars_save_string_lit("default_project_directory"));
            set_hot_directory(app, directory);
            load_project(app);
        }
    }

    {
        def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
        clear_all_layouts(app);
    }
}

function void
aleman_pre_command(Application_Links *app, Managed_Scope scope){
    Rewrite_Type *next_rewrite =
        scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
    *next_rewrite = Rewrite_None;
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_ID view_it = get_view_next(app, 0, Access_Always);
             view_it != 0;
             view_it = get_view_next(app, view_it, Access_Always)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it);
            b32 *snap_mark_to_cursor =
                scope_attachment(app, scope_it, view_snap_mark_to_cursor,
                                 b32);
            *snap_mark_to_cursor = true;
        }
    }

    if (current_editor_mode == EditorMode_Normal) {
        set_current_mapid(app, mapid_normal);
    }
}

function void
aleman_setup_essential_mapping(Mapping *mapping, i64 mapid_global, i64 mapid_file, i64 mapid_code) {
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(mapid_global);
    BindCore(custom_startup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    
    // SelectMap(file_id);
    // ParentMap(global_id);
    // BindTextInput(write_text_input);
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    // SelectMap(mapid_code);
    // ParentMap(mapid_file);
    // BindTextInput(write_text_and_auto_indent);
}
