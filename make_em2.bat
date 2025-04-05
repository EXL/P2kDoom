:: This make_ep2.bat script was created by EXL, 20-Apr-2023.
:: Default platform is Motorola P2K, ElfPack v2.x, devkitARM release 26 (GCC 4.4.0) on Windows.

:: Uncomment it for disable verbose output.
:: @echo off

if /I "%1"=="clean" (
	if exist src\*.o    del src\*.o
	if exist src\*.obj  del src\*.obj
	if exist *.elfp del *.elfp
	if exist *.elf  del *.elf
	exit /b 0
)

:: Compiler path.
set MCORE_PATH=C:\MCORE_EM2

:: SDK path.
set SDK_PATH=%MCORE_PATH%\sdk

:: Libraries path.
set LIB_PATH=%MCORE_PATH%\lib

:: Defines.
set DEFINES=-std=c99 -D__P2K__ -DEM2 -DROT_0 -DFPS_30 -DRANGECHECK -DGBA -DP2K -DUSE_BIG_ENDIAN
:: set DEFINES=-D__P2K__ -DEP2 -DROT_90 -DFPS_30 -DJAVA_HEAP
:: set DEFINES=-DDEBUG
:: set DEFINES=-DFPS_METER
:: set DEFINES=-DMEMORY_MANUAL_ALLOCATION
:: set DEFINES=-DSEARCH_LONG_RANGE

:: Includes.
set INCLUDES=-I. -Iinc

:: Optimization.
set OPTIM=-O2

:: Project/ELF name.
set ELF_NAME=P2kDoom

:: Compiling step.
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/am_map.c -o src/am_map.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/d_client.c -o src/d_client.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/d_items.c -o src/d_items.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/d_main.c -o src/d_main.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/f_finale.c -o src/f_finale.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/f_wipe.c -o src/f_wipe.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/g_game.c -o src/g_game.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/global_data.c -o src/global_data.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/hu_lib.c -o src/hu_lib.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/hu_stuff.c -o src/hu_stuff.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/i_audio.c -o src/i_audio.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/i_main.c -o src/i_main.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/i_system.c -o src/i_system.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/i_system_p2k.c -o src/i_system_p2k.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/i_video.c -o src/i_video.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/info.c -o src/info.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/lprintf.c -o src/lprintf.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/m_bbox.c -o src/m_bbox.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/m_cheat.c -o src/m_cheat.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/m_menu.c -o src/m_menu.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/m_random.c -o src/m_random.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/m_recip.c -o src/m_recip.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_ceilng.c -o src/p_ceilng.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_doors.c -o src/p_doors.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_enemy.c -o src/p_enemy.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_floor.c -o src/p_floor.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_genlin.c -o src/p_genlin.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_inter.c -o src/p_inter.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_lights.c -o src/p_lights.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_map.c -o src/p_map.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_maputl.c -o src/p_maputl.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_mobj.c -o src/p_mobj.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_plats.c -o src/p_plats.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_pspr.c -o src/p_pspr.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_setup.c -o src/p_setup.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_sight.c -o src/p_sight.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_spec.c -o src/p_spec.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_switch.c -o src/p_switch.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_telept.c -o src/p_telept.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_tick.c -o src/p_tick.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/p_user.c -o src/p_user.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_data.c -o src/r_data.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_draw.c -o src/r_draw.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_hotpath.iwram.c -o src/r_hotpath.iwram.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_main.c -o src/r_main.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_patch.c -o src/r_patch.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_plane.c -o src/r_plane.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/r_things.c -o src/r_things.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/s_sound.c -o src/s_sound.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/sounds.c -o src/sounds.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/st_gfx.c -o src/st_gfx.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/st_lib.c -o src/st_lib.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/st_stuff.c -o src/st_stuff.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/tables.c -o src/tables.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/v_video.c -o src/v_video.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/version.c -o src/version.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/w_memcache.c -o src/w_memcache.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/w_wad.c -o src/w_wad.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/wi_stuff.c -o src/wi_stuff.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/z_bmalloc.c -o src/z_bmalloc.o
%MCORE_PATH%\bin\mcore-elf-gcc -fshort-wchar -funsigned-char -fomit-frame-pointer -fno-builtin ^
	-m340 -m4align -mbig-endian -nostdinc -nostdlib -I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% ^
	-c src/z_zone.c -o src/z_zone.o

:: Linking step.
%MCORE_PATH%\bin\mcore-elf-ld -d -EB -N -r -s -Bstatic -Bsymbolic -Bsymbolic-functions ^
	--gc-sections -nostdinc -nostdlib --unresolved-symbols=report-all -z muldefs -z combreloc -z nodefaultlib ^
	src/am_map.o src/d_client.o src/d_items.o src/d_main.o src/f_finale.o src/f_wipe.o src/g_game.o ^
	src/global_data.o src/hu_lib.o src/hu_stuff.o src/i_audio.o src/i_main.o src/i_system.o ^
	src/i_video.o src/info.o src/lprintf.o src/m_bbox.o src/m_cheat.o src/m_menu.o src/m_random.o src/m_recip.o ^
	src/p_ceilng.o src/p_doors.o src/p_enemy.o src/p_floor.o src/p_genlin.o src/p_inter.o src/p_lights.o ^
	src/p_map.o src/p_maputl.o src/p_mobj.o src/p_plats.o src/p_pspr.o src/p_setup.o src/p_sight.o src/p_spec.o ^
	src/p_switch.o src/p_telept.o src/p_tick.o src/p_user.o src/r_data.o src/r_draw.o src/r_hotpath.iwram.o ^
	src/r_main.o src/r_patch.o src/r_plane.o src/r_things.o src/s_sound.o src/sounds.o src/st_gfx.o src/st_lib.o ^
	src/st_stuff.o src/tables.o src/v_video.o src/version.o src/w_memcache.o src/wi_stuff.o ^
	src/z_bmalloc.o src/z_zone.o src/w_wad.o src/i_system_p2k.o %MCORE_PATH%\mcore-elf\lib\big\libgcc.a ^
	-T%MCORE_PATH%\mcore-elf\lib\linker_elf.ld -o %ELF_NAME%.elf
