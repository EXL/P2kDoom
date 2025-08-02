CC       = gcc -m32
CFLAGS   = -DFLAT_SPAN -DC_ONLY -DSDL2
CFLAGS  += -O2 -Wall -pedantic
LDFLAGS  = -s -Wl,--gc-sections -Wl,--allow-multiple-definition
LIBS     = -lSDL2

OBJS = \
	i_sdl2.o \
	p_enemy2.o \
	p_map.o \
	p_maputl.o \
	p_mobj.o \
	p_sight.o \
	r_data.o \
	r_draw.o \
	r_plane.o \
	tables.o \
	w_wad.o \
	z_zone.o \
	am_map.o \
	d_items.o \
	d_main.o \
	f_finale.o \
	f_lib.o \
	g_game.o \
	hu_stuff.o \
	i_audio.o \
	i_main.o \
	i_system.o \
	info.o \
	m_cheat.o \
	m_menu.o \
	m_random.o \
	p_doors.o \
	p_enemy.o \
	p_floor.o \
	p_inter.o \
	p_lights.o \
	p_map.o \
	p_maputl.o \
	p_mobj.o \
	p_plats.o \
	p_pspr.o \
	p_setup.o \
	p_sight.o \
	p_spec.o \
	p_switch.o \
	p_telept.o \
	p_tick.o \
	p_user.o \
	r_sky.o \
	r_things.o \
	s_sound.o \
	sounds.o \
	st_pal.o \
	st_stuff.o \
	v_video.o \
	wi_lib.o \
	wi_stuff.o \
	z_bmallo.o

TARGET = Doom8088

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
