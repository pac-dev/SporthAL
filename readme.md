This project adapts [Soundpipe](https://paulbatchelor.github.io/proj/soundpipe.html) and [Sporth](https://paulbatchelor.github.io/proj/sporth.html) into a Premake5 project, with glue to play them through OpenAL. This allows targeting more toolchains and platforms, such as emscripten / JS and MSVC / Windows. 

## Building with emscripten
Make sure you have Premake5 and the emscripten SDK, then:

	premake5 gmake
	cd build/emscripten
	emsdk activate latest
	make config=release