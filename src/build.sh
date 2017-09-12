source ~/Downloads/emsdk/emsdk_set_env.sh
emcc js.c logic.c convert.c binary.c game.c file.c unzip.c datamap.c -o ../emerald.html --preload-file cave/40S.bin --preload-file cave/41 --preload-file cave/emeraldclub_2.S --js-library c.js -s EXPORTED_FUNCTIONS="['_file_cave_refresh','_main','_init_game']"
