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
set ARM_PATH=C:\devkitARM

:: SDK path.
set SDK_PATH=%ARM_PATH%\SDK

:: Libraries path.
set LIB_PATH=%ARM_PATH%\lib

:: Main link library.
set LIB_MAIN=std.sa

:: Libc link library.
set LIB_LIBC=libc.a

:: Defines.
set DEFINES=-std=c99 -D__P2K__ -DEP2 -DROT_0 -DFPS_30 -DRANGECHECK -DGBA -DP2K -DUSE_BIG_ENDIAN
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
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/am_map.c -o src/am_map.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/d_client.c -o src/d_client.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/d_items.c -o src/d_items.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/d_main.c -o src/d_main.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/f_finale.c -o src/f_finale.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/f_wipe.c -o src/f_wipe.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/g_game.c -o src/g_game.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/global_data.c -o src/global_data.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/hu_lib.c -o src/hu_lib.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/hu_stuff.c -o src/hu_stuff.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/i_audio.c -o src/i_audio.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/i_main.c -o src/i_main.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/i_system.c -o src/i_system.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/i_system_p2k.c -o src/i_system_p2k.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/i_video.c -o src/i_video.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/info.c -o src/info.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/lprintf.c -o src/lprintf.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/m_bbox.c -o src/m_bbox.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/m_cheat.c -o src/m_cheat.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/m_menu.c -o src/m_menu.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/m_random.c -o src/m_random.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/m_recip.c -o src/m_recip.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_ceilng.c -o src/p_ceilng.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_doors.c -o src/p_doors.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_enemy.c -o src/p_enemy.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_floor.c -o src/p_floor.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_genlin.c -o src/p_genlin.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_inter.c -o src/p_inter.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_lights.c -o src/p_lights.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_map.c -o src/p_map.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_maputl.c -o src/p_maputl.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_mobj.c -o src/p_mobj.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_plats.c -o src/p_plats.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_pspr.c -o src/p_pspr.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_setup.c -o src/p_setup.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_sight.c -o src/p_sight.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_spec.c -o src/p_spec.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_switch.c -o src/p_switch.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_telept.c -o src/p_telept.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_tick.c -o src/p_tick.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/p_user.c -o src/p_user.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_data.c -o src/r_data.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_draw.c -o src/r_draw.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_hotpath.iwram.c -o src/r_hotpath.iwram.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_main.c -o src/r_main.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_patch.c -o src/r_patch.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_plane.c -o src/r_plane.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/r_things.c -o src/r_things.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/s_sound.c -o src/s_sound.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/sounds.c -o src/sounds.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/st_gfx.c -o src/st_gfx.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/st_lib.c -o src/st_lib.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/st_stuff.c -o src/st_stuff.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/tables.c -o src/tables.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/v_video.c -o src/v_video.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/version.c -o src/version.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/w_memcache.c -o src/w_memcache.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/w_wad.c -o src/w_wad.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/wi_stuff.c -o src/wi_stuff.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/z_bmalloc.c -o src/z_bmalloc.o
%ARM_PATH%\bin\arm-eabi-gcc -c -mbig-endian -mthumb -mthumb-interwork -nostdlib ^
	-fshort-wchar -fshort-enums -fpack-struct=4 -fno-builtin -fvisibility=hidden ^
	-I%SDK_PATH% %INCLUDES% %DEFINES% %OPTIM% src/z_zone.c -o src/z_zone.o

:: Linking step.
%ARM_PATH%\bin\arm-eabi-ld -pie -EB %OPTIM% -nostdlib --allow-multiple-definition --as-needed ^
	src/am_map.o src/d_client.o src/d_items.o src/d_main.o src/f_finale.o src/f_wipe.o src/g_game.o ^
	src/global_data.o src/hu_lib.o src/hu_stuff.o src/i_audio.o src/i_main.o src/i_system.o ^
	src/i_video.o src/info.o src/lprintf.o src/m_bbox.o src/m_cheat.o src/m_menu.o src/m_random.o src/m_recip.o ^
	src/p_ceilng.o src/p_doors.o src/p_enemy.o src/p_floor.o src/p_genlin.o src/p_inter.o src/p_lights.o ^
	src/p_map.o src/p_maputl.o src/p_mobj.o src/p_plats.o src/p_pspr.o src/p_setup.o src/p_sight.o src/p_spec.o ^
	src/p_switch.o src/p_telept.o src/p_tick.o src/p_user.o src/r_data.o src/r_draw.o src/r_hotpath.iwram.o ^
	src/r_main.o src/r_patch.o src/r_plane.o src/r_things.o src/s_sound.o src/sounds.o src/st_gfx.o src/st_lib.o ^
	src/st_stuff.o src/tables.o src/v_video.o src/version.o src/w_memcache.o src/wi_stuff.o ^
	src/z_bmalloc.o src/z_zone.o src/w_wad.o src/i_system_p2k.o libgcc.a ^
	%LIB_PATH%\%LIB_MAIN% %LIB_PATH%\%LIB_LIBC% -o %ELF_NAME%.elfp

:: Post linking step.
%ARM_PATH%\libgen\postlink %ELF_NAME%.elfp -o %ELF_NAME%.elf
