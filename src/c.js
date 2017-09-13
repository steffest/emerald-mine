mergeInto(LibraryManager.library, {
	js_initScreen: function(w,h) {
		console.log('initScreen: width: ' + w + ' height: ' + h);

		var canvas = document.getElementById("canvas");
		canvas.width = w;
		canvas.height = h;

		canvas.style.width = w + "px";
		canvas.style.height = h + "px";

		window.canvas = canvas;
		window.ctx = canvas.getContext("2d");

		window.js_counter =  {};
		window.js_log = function(name,str){
			/*
				var c = window.js_counter[name] || 0;
				if (c<100) console.log(name + " " + str);
				c++;
				window.js_counter[name] = c;
			*/
		};
	},
	js_initKeyboard: function(w,h) {
		//console.log('init keyBoard');

		window.keyBoardState = {};

		document.addEventListener("keydown", function(e){
			var keyCode = e.keyCode;
			window.keyBoardState[keyCode] = 1;
			//console.log("down " +  keyCode);
		}, false);

		document.addEventListener("keyup", function(e){
			var keyCode = e.keyCode;
			window.keyBoardState[keyCode] = 0;
			//console.log("up " + keyCode);
		}, false);

	},
	js_menuClear: function() {
		Sprites.drawMenu();
	},
	js_drawMenu: function() {
		//Sprites.drawMenu();
	},
	js_createCanvas : function(canvasName,width,height){
		console.log("create Canvas " + Pointer_stringify(canvasName),width,height);
		var n = Pointer_stringify(canvasName);
		var canvasName = n + "Canvas";
		var canvas = document.createElement("canvas");
		canvas.width = width;
		canvas.height = height;

		window[canvasName] = canvas;
		window[n] = canvas.getContext("2d");
		console.log(n + " created");

		if (n == "objectPixmap"){
			window[n].fillStyle = "rgba(0,0,0,0)";
			window[n].fillRect(0,0,width,height);
			Sprites.render();
		}

	},
	js_clearRect : function(x,y,width,height){
		window.js_log("clearRect", Array.prototype.slice.call(arguments).join(","));

		var c = window.ctx;
		c.fillStyle = "black";
		c.fillRect(x,y,width,height);
	},
	js_menuPrint : function(str,top){
		//window.js_log("menuPrint", Pointer_stringify(str));
		var c = window.ctx;
		c.font = '24px courier';
		c.fillStyle = "white";
		c.fillText(Pointer_stringify(str),10,20 + (top * 24));
	},
	js_getKey : function(keyCode){
		//window.js_log("getKey", keyCode);
		return window.keyBoardState[keyCode] || 0;
	},
	js_printScore : function(str){
		//window.js_log("printScore", Pointer_stringify(str));

		var c = window.scorePixmap;
		c.clearRect(0,0,640,16);
		c.fillStyle = "rgba(0,0,0,0.5)";
		c.fillRect(0,0,640,16);
		c.font = '16px monospace';
		c.fillStyle = "white";
		var text = Pointer_stringify(str).toUpperCase();
		c.fillText(text,140,13);
	},
	js_blit: function(source,srcX,srcY,srcW,srcH,destination,dstX,dstY,dstW,dstH){
		window.js_log("blit", Array.prototype.slice.call(arguments).join(","));
		window[Pointer_stringify(destination)].drawImage(window[Pointer_stringify(source) + "Canvas"],srcX,srcY,srcW,srcH,dstX,dstY,dstW,dstH);

	},
	js_fillRect: function(color, x, y, w, h){
		window.js_log("fillRect", Array.prototype.slice.call(arguments).join(","));
		var c = window.ctx;
		c.fillStyle = Pointer_stringify(color);
		c.fillRect(x,y,w,h);
	},
	js_drawGameScreen: function(x,y,w,h){
		//window.js_log("drawGameScreen", Array.prototype.slice.call(arguments).join(","));

		var c = window.ctx;
		c.fillStyle = "black";
		c.fillRect(0,0,w,h);

		c.drawImage(window.screenPixmapCanvas,x,y,w,h,0,0,w,h);
		c.drawImage(window.scorePixmapCanvas,0,0);

		EventBus.trigger(EVENT.GAME_SCREEN_DRAWN);

	},
	js_setObjectPixelRGB: function(r,g,b,x,y){
		//note - this is only used once at development time to generate the sprites
		// after that sprites are loaded from .png

		// predefined for scale=2
		var scale = 2;
		var alpha = 1;

		// make transparent after column 29: player sprites
		if (x>448 && ((r+b+g)==0)) alpha = 0;

		window.objectPixmap.fillStyle = "rgba("+r+","+g+","+ b +","+ alpha +")";
		window.objectPixmap.fillRect( x*scale, y*scale, scale, scale );
	},
	js_playAudio: function(index){
		Audio.playSample(index);
	},
	js_quit : function(){
		window.location.href = "http://www.emeraldmines.net";
	},
	js_event: function(event,data){
		EventBus.trigger(event,data);
	},
	js_updateScore: function(score,diamonds,time){
		Game.score = score;
		Game.diamonds = diamonds;
		Game.time = time;
	}
});

