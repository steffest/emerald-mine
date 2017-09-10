source ~/Downloads/emsdk/emsdk_set_env.sh
emcc js.c logic.c convert.c binary.c game.c file.c unzip.c datamap.c -o ../emerald.html --preload-file cave/emerald_mine_1.S --preload-file cave/emeraldclub_2.S --js-library c.js
