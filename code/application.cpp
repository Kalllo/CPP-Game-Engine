/*

    This is the actual implementation layer for the application
    you write UI / Game code starting in this layer.
    
    The platform that you compile to will call into this layer
    and this layer will respond with releveant information 
    to get the platform layer to do things. Meaning that
    this layer is filled with platform and archetecture
    agnostic code.

    Author: Justin Morrow
*/

#include "application.h"

// output sound from the
internal void ApplicationOutputSound(application_sound_output_buffer *sound_buffer, int tone_hz)
{
    local_persist real32 t_sine;
    int16 tone_volume = 3000;
    int wave_period = sound_buffer->SamplesPerSecond/tone_hz;

    int16 *sample_out = sound_buffer->Samples;
    for(int sample_idx = 0;
        sample_idx < sound_buffer->SampleCount;
        ++sample_idx)
    {
        // TODO: Draw this out for people
        real32 sine_value = sinf(t_sine);
        int16 sample_value = (int16)(sine_value * tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        t_sine += 2.0f*Pi32*1.0f/(real32)wave_period;
    }
}

// render a cool blue, green gradient
void RenderWeirdGradient(offscreen_graphics_buffer *buffer, int x_offset, int y_offset)
{
    uint8 *row = (uint8 *)buffer->Memory;
    for (int y = 0; y < buffer->Height; ++y) 
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < buffer->Width; ++x) 
        {
            

            uint8 blue = (uint8)(x + x_offset);
            uint8 green = (uint8)(y + y_offset);
            *pixel++ = (green << 8) | blue;
        }

        row += buffer->Pitch;
    }
}

// the main application update loop
// all platform non-specific code gets executed here
APP_UPDATE_AND_RENDER(AppUpdateAndRender)
{
    Assert(sizeof(application_state) <= memory->PermanentStorageSize);
    
    // this app_state is how you access the application state from permanent storage
    // so you can call this in many other functions and it will retreive the game state
    application_state *app_state = (application_state *)memory->PermanentStorage;
    if(!memory->IsInitialized)
    {
        char *filename = __FILE__;
        
        debug_read_file_result file = DEBUGPlatformReadEntireFile(filename);
        if(file.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", file.ContentsSize, file.Contents);
            DEBUGPlatformFreeFileMemory(file.Contents);
        }
        
        app_state->ToneHz = 256;
        app_state->BlueOffset = 0;
        app_state->GreenOffset = 0;

        // TODO: This may be more appropriate to do in the platform layer
        memory->IsInitialized = true;
    }

    for (int controller_idx = 0; controller_idx < ArrayCount(input->Controllers); ++controller_idx)
    {
        application_controller_input *controller = GetController(input, controller_idx);
    
        if(controller->IsAnalog)
        {
            real32 x_offset_speed = 4.0f;
            real32 y_offset_speed = 128.0f;
            
            app_state->BlueOffset += (int)(x_offset_speed*(controller->StickAverageX));
            app_state->ToneHz = 256 + (int)(y_offset_speed*(controller->StickAverageY));
        }
        else
        {
            if(controller->MoveLeft.EndedDown)
            {
                app_state->BlueOffset -= 1;
            }
            if(controller->MoveRight.EndedDown)
            {
                app_state->BlueOffset += 1;
            }
            // NOTE: Use digital movement tuning
        }

        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;
        if(controller->ActionDown.EndedDown)
        {
            app_state->GreenOffset += 1;
        }
    }

    RenderWeirdGradient(buffer, app_state->BlueOffset, app_state->GreenOffset);
}

APP_GET_SOUND_SAMPLES(AppGetSoundSamples)
{
    application_state *app_state = (application_state *)memory->PermanentStorage;
    ApplicationOutputSound(sound_buffer, app_state->ToneHz);
}
