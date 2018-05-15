/* 
 * sporthal.cpp
 * Play Sporth scripts through the OpenAL API.
 */

extern "C" {
#include "soundpipe.h"
#include "sporth.h"
#include "plumber.h"
}
#include "soundpipeal.h"


static void process(sp_data *sp, void *udp)
{
	plumber_data *pd = (plumber_data *)udp;
	plumber_compute(pd, PLUMBER_COMPUTE);
	SPFLOAT out;
	sp->out[0] = sporth_stack_pop_float(&pd->sporth.stack);
}

sp_data *sp;
plumber_data pd;
SoundpipeALState *spal;

extern "C" {
	int sporthal_init()
	{
		sp = 0;
		return 0;
	}

	int sporthal_compile(char *script)
	{
		if (sp) {
			plumber_recompile_string(&pd, script);
			return 0;
		}
		sp_create(&sp);
		plumber_register(&pd);
		plumber_init(&pd);
		pd.sp = sp;
		plumber_parse_string(&pd, script);

		//pd.p[0] = 440; // p-register test

		plumber_compute(&pd, PLUMBER_INIT);
		spal = soundpipeal_init(pd.sp, &pd, process);

		// functions must return immediately for emscripten compatibility
		return 0;
	}
	
	int sporthal_start()
	{
		printf("starting audio\n");
		soundpipeal_start(spal);
		return 0;
	}
	
	int sporthal_stop()
	{
		printf("stopping audio\n");
		soundpipeal_stop();
		return 0;
	}
}