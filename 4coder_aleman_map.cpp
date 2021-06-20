
CUSTOM_COMMAND_SIG(to_normal_mode) {
    set_current_mapid(app, mapid_normal);
    current_editor_mode = EditorMode_Normal;
}

CUSTOM_COMMAND_SIG(to_visual_mode) {
    set_current_mapid(app, mapid_visual);
    current_editor_mode = EditorMode_Visual;
    set_mark(app);
}

CUSTOM_COMMAND_SIG(to_leader_mode) {
    set_current_mapid(app, mapid_leader);
}

CUSTOM_COMMAND_SIG(to_i_keymap) {
    set_current_mapid(app, mapid_i_keymap);
}

CUSTOM_COMMAND_SIG(to_k_keymap) {
    set_current_mapid(app, mapid_k_keymap);
}

CUSTOM_COMMAND_SIG(to_d_keymap) {
    set_current_mapid(app, mapid_d_keymap);
}

CUSTOM_COMMAND_SIG(to_insert_mode) {
    set_current_mapid(app, mapid_insert);
    current_editor_mode = EditorMode_Insert;
}

CUSTOM_COMMAND_SIG(project_f1key_command) {
    prj_exec_command_fkey_index(app, 0);
}

function void
seek_pos_of_visual_line_or_blank_line(Application_Links *app, Side side,
                                       Scan_Direction direction, Position_Within_Line position) {
    View_ID view = get_active_view(app, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p = view_relative_xy_of_pos(app, view, cursor.line, pos);
    p.x = (side == Side_Min)?(0.f):(max_f32);
    i64 new_pos = view_pos_at_relative_xy(app, view, cursor.line, p);

    if (new_pos == pos) {
        if (pos > 0) {
            seek_blank_line(app, direction, position);
        }
    } else {
        view_set_cursor_and_preferred_x(app, view, seek_pos(new_pos));
        no_mark_snap_to_cursor_if_shift(app, view);
    }
}

CUSTOM_COMMAND_SIG(seek_beginning_of_line_or_block) {
    seek_pos_of_visual_line_or_blank_line(app, Side_Min, Scan_Backward, PositionWithinLine_End);
}

CUSTOM_COMMAND_SIG(seek_end_of_line_or_block) {
    seek_pos_of_visual_line_or_blank_line(app, Side_Max, Scan_Forward, PositionWithinLine_Start);
}

CUSTOM_COMMAND_SIG(insert_newline) {
    write_text(app, string_u8_litexpr("\n"));
}

CUSTOM_COMMAND_SIG(append_newline) {
    insert_newline(app);
    move_left(app);
}

function void
aleman_setup_modal_mapping(Mapping *mapping, i64 mapid_global, i64 mapid_file, i64 mapid_code) {
    MappingScope();
    SelectMapping(mapping);

    SelectMap(mapid_global);

    // NOTE(alexander): Keys shared for both insert and normal mode
    SelectMap(mapid_shared);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(word_complete, KeyCode_Tab);
    Bind(delete_char, KeyCode_Delete);
    Bind(backspace_char, KeyCode_Backspace);
    Bind(move_left, KeyCode_Left);
    Bind(move_right, KeyCode_Right);
    Bind(move_up, KeyCode_Up);
    Bind(move_down, KeyCode_Down);
    Bind(exit_4coder, KeyCode_F4, KeyCode_Alt);
    Bind(project_fkey_command, KeyCode_F1);
    Bind(project_fkey_command, KeyCode_F2);
    Bind(project_fkey_command, KeyCode_F3);
    Bind(project_fkey_command, KeyCode_F4);
    Bind(project_fkey_command, KeyCode_F5);
    Bind(project_fkey_command, KeyCode_F6);
    Bind(project_fkey_command, KeyCode_F7);
    Bind(project_fkey_command, KeyCode_F8);
    Bind(project_fkey_command, KeyCode_F9);
    Bind(project_fkey_command, KeyCode_F10);
    Bind(project_fkey_command, KeyCode_F11);
    Bind(project_fkey_command, KeyCode_F12);
    Bind(project_fkey_command, KeyCode_F13);
    Bind(project_fkey_command, KeyCode_F14);
    Bind(project_fkey_command, KeyCode_F15);
    Bind(project_fkey_command, KeyCode_F16);

    // NOTE(alexander): Keys for normal mode
    SelectMap(mapid_normal);
    ParentMap(mapid_shared);
    Bind(to_leader_mode, KeyCode_Space);
    Bind(to_leader_mode, KeyCode_Backspace);
    Bind(to_insert_mode, KeyCode_F);
    Bind(to_visual_mode, KeyCode_T);
    Bind(insert_newline, KeyCode_Return);
    // Bind(if_read_only_goto_position,  KeyCode_Return);
    Bind(close_panel, KeyCode_3);
    Bind(open_panel_hsplit, KeyCode_4);
    Bind(command_lister, KeyCode_A);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_E);
    Bind(snipe_forward_whitespace_or_token_boundary, KeyCode_R);
    Bind(delete_char, KeyCode_5);
    Bind(delete_line, KeyCode_G);
    Bind(backspace_char, KeyCode_D);
    Bind(center_view,  KeyCode_U);
    Bind(move_up, KeyCode_O);
    Bind(move_left, KeyCode_K);
    Bind(move_down, KeyCode_L);
    Bind(move_right, KeyCode_Semicolon);
    Bind(move_left_alpha_numeric_or_camel_boundary,  KeyCode_I);
    Bind(move_right_alpha_numeric_or_camel_boundary, KeyCode_P);
    Bind(goto_prev_jump, KeyCode_H);
    Bind(seek_beginning_of_line_or_block,  KeyCode_J);
    Bind(seek_end_of_line_or_block, KeyCode_Quote);
    Bind(goto_next_jump, KeyCode_BackwardSlash);
    Bind(comment_line_toggle, KeyCode_Z);
    Bind(append_newline,  KeyCode_S);
    Bind(cut, KeyCode_X);
    Bind(copy, KeyCode_C);
    Bind(paste, KeyCode_V);
    Bind(undo, KeyCode_Y);
    Bind(redo, KeyCode_B);
    Bind(search, KeyCode_N);
    Bind(change_active_panel, KeyCode_Comma);
    Bind(change_active_panel_backwards, KeyCode_Period);

    // NOTE(alexander): Keys for visual mode
    SelectMap(mapid_visual);
    ParentMap(mapid_normal);

    // NOTE(alexander): Keys available after pressing leader key
    SelectMap(mapid_leader);
    ParentMap(mapid_global);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(to_insert_mode, KeyCode_Backspace);
    Bind(to_insert_mode, KeyCode_Space);
    Bind(to_i_keymap, KeyCode_I);
    Bind(to_k_keymap, KeyCode_K);
    Bind(to_d_keymap, KeyCode_D);
    
    Bind(close_panel, KeyCode_3);
    Bind(open_panel_vsplit, KeyCode_4);
    Bind(select_all, KeyCode_A);
    Bind(query_replace, KeyCode_R);
    Bind(goto_beginning_of_file, KeyCode_H);
    Bind(goto_end_of_file, KeyCode_N);
    Bind(project_f1key_command, KeyCode_P);
    Bind(save_all_dirty_buffers, KeyCode_Semicolon);
    BindTextInput(to_normal_mode);

    // NOTE(alexander): Keys for i keymap
    SelectMap(mapid_i_keymap);
    ParentMap(mapid_global);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(interactive_open_or_new, KeyCode_E);
    Bind(interactive_switch_buffer, KeyCode_D);
    Bind(open_matching_file_cpp, KeyCode_C);
    BindTextInput(to_normal_mode);

     // NOTE(alexander): Keys for k keymap
    SelectMap(mapid_k_keymap);
    ParentMap(mapid_global);
    Bind(to_normal_mode, KeyCode_Escape);
    Bind(kill_buffer, KeyCode_J);
    Bind(open_matching_file_cpp, KeyCode_C);
    Bind(goto_line, KeyCode_Y);
    Bind(query_replace_identifier, KeyCode_R);
    Bind(query_replace_selection, KeyCode_T);
    BindTextInput(to_normal_mode);

    SelectMap(mapid_d_keymap);
    ParentMap(mapid_global);
    Bind(write_todo, KeyCode_T);
    Bind(write_note, KeyCode_N);
    BindTextInput(to_normal_mode);
 
    // NOTE(alexander): Keys for insert mode
    SelectMap(mapid_insert);
    ParentMap(mapid_shared);
    BindTextInput(write_text_and_auto_indent);
    Bind(to_normal_mode, KeyCode_Home);
}
