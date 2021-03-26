
CUSTOM_COMMAND_SIG(aleman_view_input_handler)
CUSTOM_DOC("Input consumption loop for default view behavior")
{
    Scratch_Block scratch(app);
    default_input_handler_init(app, scratch);
    
    View_ID view = get_this_ctx_view(app, Access_Always);
    Managed_Scope scope = view_get_managed_scope(app, view);
    
    for (;;){
        // NOTE(allen): Get input
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        if (input.abort){
            break;
        }
        
        ProfileScopeNamed(app, "before view input", view_input_profile);
        
        // NOTE(allen): Mouse Suppression
        Event_Property event_properties = get_event_properties(&input.event);
        if (suppressing_mouse && (event_properties & EventPropertyGroup_AnyMouseEvent) != 0){
            continue;
        }
        
        // NOTE(allen): Get binding
        if (implicit_map_function == 0){
            implicit_map_function = default_implicit_map;
        }
        Implicit_Map_Result map_result = implicit_map_function(app, 0, 0, &input.event);
        if (map_result.command == 0){
            leave_current_input_unhandled(app);
            continue;
        }
        
        // NOTE(allen): Run the command and pre/post command stuff
        aleman_pre_command(app, scope);
        ProfileCloseNow(view_input_profile);
        map_result.command(app);
        ProfileScope(app, "after view input");
        default_post_command(app, scope);
    }
}

function void
aleman_draw_normal_mode_cursor(Application_Links *app, View_ID view_id, b32 is_active_view,
                               Buffer_ID buffer, Text_Layout_ID text_layout_id,
                               f32 roundness, f32 outline_thickness) {
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    View_ID active_view_id = get_active_view(app, Access_ReadVisible);
    if (!has_highlight_range && active_view_id == view_id) {
        i32 cursor_sub_id = default_cursor_sub_id();
        
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        if (is_active_view){
            draw_character_block(app, text_layout_id, cursor_pos, roundness,
                                 fcolor_id(defcolor_cursor, cursor_sub_id));
            paint_text_color_pos(app, text_layout_id, cursor_pos,
                                 fcolor_id(defcolor_at_cursor));
        }
        else{
            draw_character_wire_frame(app, text_layout_id, cursor_pos,
                                      roundness, outline_thickness,
                                      fcolor_id(defcolor_cursor, cursor_sub_id));
        }
    }
}

function void
aleman_draw_insert_mode_cursor(Application_Links *app, View_ID view_id,
                               Buffer_ID buffer, Text_Layout_ID text_layout_id,
                               f32 roundness) {
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    View_ID active_view_id = get_active_view(app, Access_ReadVisible);
    if (!has_highlight_range && active_view_id == view_id) {
        i32 cursor_sub_id = default_cursor_sub_id();
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        ARGB_Color color = fcolor_resolve(fcolor_id(defcolor_cursor, cursor_sub_id));
        Rect_f32 rect = text_layout_character_on_screen(app, text_layout_id, cursor_pos);
        rect.x1 = rect.x0 + 3.0f;
        draw_rectangle(app, rect, 0.0f, color);
    }
}

function void
aleman_draw_visual_mode_cursor_highlight(Application_Links *app, View_ID view_id, b32 is_active_view,
                                          Buffer_ID buffer, Text_Layout_ID text_layout_id,
                                          f32 roundness, f32 outline_thickness) {
    b32 has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, roundness);
    View_ID active_view_id = get_active_view(app, Access_ReadVisible);
    if (!has_highlight_range && active_view_id == view_id) {
        i64 cursor_pos = view_get_cursor_pos(app, view_id);
        i64 mark_pos = view_get_mark_pos(app, view_id);
        if (cursor_pos != mark_pos){
            Range_i64 range = Ii64(cursor_pos, mark_pos);
            draw_character_block(app, text_layout_id, range, roundness, fcolor_id(defcolor_highlight));
            paint_text_color_fcolor(app, text_layout_id, range, fcolor_id(defcolor_at_highlight));
        }
    }
    
    aleman_draw_normal_mode_cursor(app, view_id, is_active_view, buffer,
                                   text_layout_id, roundness, outline_thickness);
}

function void
aleman_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                      Buffer_ID buffer, Text_Layout_ID text_layout_id,
                      Rect_f32 rect){
    ProfileScope(app, "render buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
    f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
    f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        draw_cpp_token_colors(app, text_layout_id, &token_array);
        
        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keyword = def_get_config_b32(vars_save_string_lit("use_comment_keyword"));
        if (use_comment_keyword){
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
                {string_u8_litexpr("FIXME"), finalize_color(defcolor_comment_pop, 2)},
                {string_u8_litexpr("HACK"), finalize_color(defcolor_comment_pop, 3)},
                {string_u8_litexpr("REVIEW"), finalize_color(defcolor_comment_pop, 4)},
                {string_u8_litexpr("DEPRECATED"), finalize_color(defcolor_comment_pop, 5)},
                {string_u8_litexpr("BUG"), finalize_color(defcolor_comment_pop, 6)},
            };
            draw_comment_highlights(app, buffer, text_layout_id, &token_array, pairs, ArrayCount(pairs));
        }
        
#if 0
        // TODO(allen): Put in 4coder_draw.cpp
        // NOTE(allen): Color functions
        
        Scratch_Block scratch(app);
        ARGB_Color argb = 0xFFFF00FF;
        
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, visible_range.first);
        for (;;){
            if (!token_it_inc_non_whitespace(&it)){
                break;
            }
            Token *token = token_it_read(&it);
            String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, token);
            Code_Index_Note *note = code_index_note_from_string(lexeme);
            if (note != 0 && note->note_kind == CodeIndexNote_Function){
                paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb);
            }
        }
#endif
    }
    else{
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
    if (use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
    b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
    if (use_error_highlight || use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
    if (use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(allen): Line highlight
    b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
    }
    
    // NOTE(allen): Whitespace highlight
    b64 show_whitespace = false;
    view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
    if (show_whitespace){
        if (token_array.tokens == 0){
            draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
        }
        else{
            draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
        }
    }
    
    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            if (current_editor_mode == EditorMode_Insert) {
                aleman_draw_insert_mode_cursor(app, view_id, buffer, text_layout_id, cursor_roundness);
            } else if (current_editor_mode == EditorMode_Visual) {
                aleman_draw_visual_mode_cursor_highlight(app, view_id, is_active_view, buffer,
                                                         text_layout_id, cursor_roundness, mark_thickness);
            } else {
                aleman_draw_normal_mode_cursor(app, view_id, is_active_view, buffer,
                                               text_layout_id, cursor_roundness, mark_thickness);
            }
        }break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        }break;
    }
    
    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}

function void
aleman_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "default render caller");
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view, 1.0f);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        draw_file_bar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                  frame_info.animation_dt, scroll);
    if (!block_match_struct(&scroll.position, &delta.point)){
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    region = default_draw_query_bars(app, region, view_id, face_id);
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
    Rect_f32 line_number_rect = {};
    if (show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    aleman_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);

    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

internal void
aleman_set_all_default_hooks(Application_Links *app){
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
    
    set_custom_hook(app, HookID_ViewEventHandler, aleman_view_input_handler);
    set_custom_hook(app, HookID_Tick, default_tick);
    // set_custom_hook(app, HookID_RenderCaller, default_render_caller);
    set_custom_hook(app, HookID_RenderCaller, aleman_render_caller);
    set_custom_hook(app, HookID_WholeScreenRenderCaller, default_whole_screen_render_caller);
    
    set_custom_hook(app, HookID_DeltaRule, fixed_time_cubic_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(fixed_time_cubic_delta_memory_size));
    
    set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
    
    set_custom_hook(app, HookID_BeginBuffer, default_begin_buffer);
    set_custom_hook(app, HookID_EndBuffer, end_buffer_close_jump_list);
    set_custom_hook(app, HookID_NewFile, default_new_file);
    set_custom_hook(app, HookID_SaveFile, default_file_save);
    set_custom_hook(app, HookID_BufferEditRange, default_buffer_edit_range);
    set_custom_hook(app, HookID_BufferRegion, default_buffer_region);
    set_custom_hook(app, HookID_ViewChangeBuffer, default_view_change_buffer);
    
    set_custom_hook(app, HookID_Layout, layout_unwrapped);
    //set_custom_hook(app, HookID_Layout, layout_wrap_anywhere);
    //set_custom_hook(app, HookID_Layout, layout_wrap_whitespace);
    //set_custom_hook(app, HookID_Layout, layout_virt_indent_unwrapped);
    //set_custom_hook(app, HookID_Layout, layout_unwrapped_small_blank_lines);
}
