#include "4coder_default_include.cpp"


// TODO(alexander): this is defined here for now
enum {
    EditorMode_Normal = 0,
    EditorMode_Insert = 1,
    EditorMode_Visual = 2,
    EditorMode_Leader = 3,
};
global i32 current_editor_mode = EditorMode_Normal;

// NOTE(alexander): define custom binding maps for modal editing
CUSTOM_ID(command_map, mapid_shared);
CUSTOM_ID(command_map, mapid_normal);
CUSTOM_ID(command_map, mapid_insert);
CUSTOM_ID(command_map, mapid_visual);
CUSTOM_ID(command_map, mapid_leader);
CUSTOM_ID(command_map, mapid_i_keymap);
CUSTOM_ID(command_map, mapid_k_keymap);
CUSTOM_ID(command_map, mapid_w_keymap);

void
set_current_mapid(Application_Links* app, Command_Map_ID mapid) {
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = mapid;
}

#include "4coder_aleman_framework.cpp"
#include "4coder_aleman_hooks.cpp"
#include "4coder_aleman_map.cpp"

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

void
custom_layer_init(Application_Links *app) {
    Thread_Context *tctx = get_thread_context(app);

    // NOTE(allen): setup for default framework
    default_framework_init(app);

    // NOTE(alexander): my custom hooks
    aleman_set_all_default_hooks(app);

    mapping_init(tctx, &framework_mapping);

    String_ID mapid_global = vars_save_string_lit("keys_global");
    String_ID mapid_file = vars_save_string_lit("keys_file");
    String_ID mapid_code = vars_save_string_lit("keys_code");

    // NOTE(alexader): use modal mapping
    aleman_setup_modal_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);

    // NOTE(alexader): use default (non-modal) mapping
    // setup_default_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);

    aleman_setup_essential_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);

    
}
