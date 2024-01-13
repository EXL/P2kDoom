QT += core gui widgets

CONFIG -= app_bundle
CONFIG -= force_debug_info

INCLUDEPATH += inc

DEFINES += RANGECHECK

SOURCES += \
#	src/fixeddiv.s \
	src/am_map.c \
	src/d_client.c \
	src/d_items.c \
	src/d_main.c \
	src/doom_iwad.c \
	src/f_finale.c \
	src/f_wipe.c \
	src/g_game.c \
	src/global_data.c \
	src/hu_lib.c \
	src/hu_stuff.c \
	src/i_audio.c \
	src/i_main.c \
	src/i_system.c \
	src/i_system_e32.cpp \
	src/i_system_gba.cpp \
	src/i_video.c \
	src/info.c \
	src/lprintf.c \
	src/m_bbox.c \
	src/m_cheat.c \
	src/m_menu.c \
	src/m_random.c \
	src/m_recip.c \
	src/p_ceilng.c \
	src/p_doors.c \
	src/p_enemy.c \
	src/p_floor.c \
	src/p_genlin.c \
	src/p_inter.c \
	src/p_lights.c \
	src/p_map.c \
	src/p_maputl.c \
	src/p_mobj.c \
	src/p_plats.c \
	src/p_pspr.c \
	src/p_setup.c \
	src/p_sight.c \
	src/p_spec.c \
	src/p_switch.c \
	src/p_telept.c \
	src/p_tick.c \
	src/p_user.c \
	src/r_data.c \
	src/r_draw.c \
	src/r_hotpath.iwram.c \
	src/r_main.c \
	src/r_patch.c \
	src/r_plane.c \
	src/r_things.c \
	src/s_sound.c \
	src/sounds.c \
	src/st_gfx.c \
	src/st_lib.c \
	src/st_stuff.c \
	src/tables.c \
	src/v_video.c \
	src/version.c \
	src/w_wad.c \
	src/wi_stuff.c \
	src/z_bmalloc.c \
	src/z_zone.c

HEADERS += \
	inc/stbar.h \
	inc/am_map.h \
	inc/config.h \
	inc/d_englsh.h \
	inc/d_event.h \
	inc/d_items.h \
	inc/d_main.h \
	inc/d_net.h \
	inc/d_player.h \
	inc/d_think.h \
	inc/d_ticcmd.h \
	inc/doom_iwad.h \
	inc/doomdata.h \
	inc/doomdef.h \
	inc/doomstat.h \
	inc/doomtype.h \
	inc/dstrings.h \
	inc/f_finale.h \
	inc/f_wipe.h \
	inc/g_game.h \
	inc/gba_functions.h \
	inc/global_data.h \
	inc/global_init.h \
	inc/hu_lib.h \
	inc/hu_stuff.h \
	inc/i_main.h \
	inc/i_network.h \
	inc/i_sound.h \
	inc/i_system.h \
	inc/i_system_e32.h \
	inc/i_system_win.h \
	inc/i_video.h \
	inc/info.h \
	inc/lprintf.h \
	inc/m_bbox.h \
	inc/m_cheat.h \
	inc/m_fixed.h \
	inc/m_menu.h \
	inc/m_misc.h \
	inc/m_random.h \
	inc/m_recip.h \
	inc/m_swap.h \
	inc/p_enemy.h \
	inc/p_inter.h \
	inc/p_map.h \
	inc/p_maputl.h \
	inc/p_mobj.h \
	inc/p_pspr.h \
	inc/p_setup.h \
	inc/p_spec.h \
	inc/p_tick.h \
	inc/p_user.h \
	inc/protocol.h \
	inc/r_data.h \
	inc/r_defs.h \
	inc/r_draw.h \
	inc/r_main.h \
	inc/r_patch.h \
	inc/r_plane.h \
	inc/r_segs.h \
	inc/r_sky.h \
	inc/r_state.h \
	inc/r_things.h \
	inc/s_sound.h \
	inc/sounds.h \
	inc/st_gfx.h \
	inc/st_lib.h \
	inc/st_stuff.h \
	inc/tables.h \
	inc/v_video.h \
	inc/version.h \
	inc/w_wad.h \
	inc/wi_stuff.h \
	inc/z_bmalloc.h \
	inc/z_zone.h
