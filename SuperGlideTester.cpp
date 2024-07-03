#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <Xinput.h>

unsigned int const FRAME_RATE = 240;
int const KEYBOARD_CROUCH_KEY = 'C';
int const GAMEPAD_CROUCH_KEY = XINPUT_GAMEPAD_LEFT_SHOULDER;
int const KEYBOARD_JUMP_KEY = ' ';
int const GAMEPAD_JUMP_KEY = XINPUT_GAMEPAD_RIGHT_SHOULDER;

// src: https://github.com/muckelba/superglidetrainer/blob/main/src/routes/superglidetrainer/%2Bpage.svelte
enum State {
	Ready, // Initial State
	Jump, // Partial Sequence
	JumpWarned, // Multi-Jump Warning Sent
	Crouch // Incorrect Sequence, let it play out for a bit
};

int main(int argc, char* argv[]) {
	LARGE_INTEGER frequency, time_start, time_now;
	DWORD dwResult;
	XINPUT_STATE xinput_state;
	State state = Ready, lastState = Jump;
	double elapsed_time = 0, elapsed_frame = 0, sg_chance = 0;
	bool crouch_pressed = false, jump_pressed = false, now_crouch_pressed = false, now_jump_pressed = false;
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time_start);
	while (true) {
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++){
			ZeroMemory(&xinput_state, sizeof(XINPUT_STATE));
			dwResult = XInputGetState(i, &xinput_state);
			if (dwResult == ERROR_SUCCESS) {
				now_crouch_pressed = xinput_state.Gamepad.wButtons & GAMEPAD_CROUCH_KEY;
				now_jump_pressed = xinput_state.Gamepad.wButtons & GAMEPAD_JUMP_KEY;
				break;
			}
		}
		now_crouch_pressed = now_crouch_pressed || (GetKeyState(KEYBOARD_CROUCH_KEY) & 0x8000);
		now_jump_pressed = now_jump_pressed || (GetKeyState(KEYBOARD_JUMP_KEY) & 0x8000);
		if (!crouch_pressed && now_crouch_pressed) {
			if (state == Ready) {
				QueryPerformanceCounter(&time_start);
				state = Crouch;
			}
			else if (state == Jump || state == JumpWarned) {
				QueryPerformanceCounter(&time_now);
				elapsed_time = (time_now.QuadPart - time_start.QuadPart) * 1000.0 / frequency.QuadPart;
				elapsed_frame = elapsed_time / (1000 / FRAME_RATE);
				if (elapsed_frame < 1) {
					sg_chance = elapsed_frame;
				}
				else if (elapsed_frame < 2) {
					sg_chance = 2 - elapsed_frame;
				}
				else {
					sg_chance = 0;
				}
				printf("%10.4fms, %10.4ff, %10.4f%%\n", elapsed_time, elapsed_frame, sg_chance * 100);
				state = Ready;
			}
			else if (state == Crouch) {
				state = Ready;
			}
		}
		if (!jump_pressed && now_jump_pressed) {
			if (state == Ready) {
				QueryPerformanceCounter(&time_start);
				state = Jump;
			}
			else if (state == Jump || state == JumpWarned) {
				QueryPerformanceCounter(&time_start);
				state = JumpWarned;
			}
			else if (state == Crouch) {
				QueryPerformanceCounter(&time_now);
				elapsed_time = (time_now.QuadPart - time_start.QuadPart) * 1000.0 / frequency.QuadPart;
				elapsed_frame = elapsed_time / (1000 / FRAME_RATE);
				sg_chance = 0;
				printf("%10.4fms, %10.4ff, %10.4f%%\n", -elapsed_time, -elapsed_frame, sg_chance * 100);
				state = Ready;
			}
		}
		crouch_pressed = now_crouch_pressed;
		jump_pressed = now_jump_pressed;
	}
	return EXIT_SUCCESS;
}