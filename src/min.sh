source ~/Downloads/emsdk/emsdk_set_env.sh
emcc -O3 js.c logic.c convert.c binary.c game.c file.c unzip.c datamap.c -o emerald.html --preload-file cave/emerald_mine_1 --preload-file cave/emerald_mine_2 --preload-file cave/emerald_mine_3 --preload-file cave/emeraldclub_2 --js-library c.js
