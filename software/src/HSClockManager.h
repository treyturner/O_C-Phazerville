// Copyright (c) 2018, Jason Justian
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// A "tick" is one ISR cycle, which happens 16666.667 times per second, or a million
// times per minute. A "tock" is a metronome beat.

#pragma once

#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include "OC_core.h"
#include "HSMIDI.h"
#include "HSUtils.h"
#include <functional>
#include <vector>

namespace HS {

static constexpr uint16_t CLOCK_TEMPO_MIN = 1;
static constexpr uint16_t CLOCK_TEMPO_MAX = 0xFFFF;
static constexpr uint32_t CLOCK_TICKS_MIN = 1000000 / CLOCK_TEMPO_MAX;
static constexpr uint32_t CLOCK_TICKS_MAX = 1000000 / CLOCK_TEMPO_MIN;

constexpr int MIDI_CLOCK_PPQN = 2;
constexpr int MIDI_OUT_PPQN = 24;
constexpr int CLOCK_MAX_MULTIPLE = 24;
constexpr int CLOCK_MIN_MULTIPLE = -31; // becomes /32

class ClockManager {
public:
    enum ClockOutput {
        LEFT_CLOCK1,
        LEFT_CLOCK2,
        RIGHT_CLOCK1,
        RIGHT_CLOCK2,
        LEFT2_CLOCK1,
        LEFT2_CLOCK2,
        RIGHT2_CLOCK1,
        RIGHT2_CLOCK2,
        MIDI_CLOCK,
        ONE_BEAT_CLOCK,
        SIXTEENTH_CLOCK,
        NR_OF_CLOCKS
    };

    uint16_t tempo; // effective BPM, for display somewhere else
    uint16_t tempo_setting;
    uint32_t ticks_per_beat; // Based on the selected tempo in BPM
    bool running = 0; // Specifies whether the clock is running for interprocess communication
    bool paused = 0; // Specifies whethr the clock is paused
    bool auto_reset = 0; // on clock start
    bool midi_out_enabled = 1;

    bool tickno = 0;
    bool auto_stop = true;
    bool extsync = false; // locked into an external clock; will stop after timeout
    uint32_t clock_tick[2] = {0,0}; // previous ticks when a physical clock was received on DIGITAL 1
    uint32_t beat_tick = 0; // The tick to count from
    uint32_t beat_count = 0;
    bool tock[NR_OF_CLOCKS] = {0,0,0,0,0,0,0,0,0,0,0}; // The current tock value
    int8_t tocks_per_beat[NR_OF_CLOCKS] = {0,0, 0,0, 0,0, 0,0, MIDI_OUT_PPQN, 1, 4}; // Multiplier
    int count[NR_OF_CLOCKS] = {0,0,0,0, 0,0,0,0, 0,0,0}; // Multiple counter, 0 is a special case when first starting the clock
    int8_t shuffle = 0; // 0 to 100
    int8_t shuffle_setting = 0;

    int clock_ppqn = 4; // external clock multiple
    bool cycle = 0; // Alternates for each tock, for display purposes

    bool boop[8] = {0,0,0,0,0,0,0,0}; // Manual triggers

    std::queue<Task> syncfn_queue;

    ClockManager() {
        SetTempoBPM(120);
    }

    void EnableMIDIOut() { midi_out_enabled = 1; }
    void DisableMIDIOut() { midi_out_enabled = 0; }

    void SetMultiply(int multiply, int ch = 0) {
        multiply = constrain(multiply, CLOCK_MIN_MULTIPLE, CLOCK_MAX_MULTIPLE);
        tocks_per_beat[ch] = multiply;
    }

    // adjusts the expected clock multiple for external clock pulses
    void SetClockPPQN(int clkppqn) {
        clock_ppqn = constrain(clkppqn, 0, 24);
    }

    /* Set ticks per tock, based on one million ticks per minute divided by beats per minute.
     * This is approximate, because the arithmetical value is likely to be fractional, and we
     * need to live with a certain amount of imprecision here. So I'm not even rounding up.
     */
    void SetTempoBPM(int bpm) {
        bpm = constrain(bpm, CLOCK_TEMPO_MIN, CLOCK_TEMPO_MAX);
        ticks_per_beat = 1000000 / bpm;
        tempo_setting = tempo = bpm;
    }
    
    void SetTempoFromTaps(uint32_t *taps, int count) {
        uint32_t total = 0;
        for (int i = 0; i < count; ++i) {
            total += taps[i];
        }
        
        // update the tempo
        uint32_t clock_diff = total / count;
        ticks_per_beat = constrain(clock_diff, CLOCK_TICKS_MIN, CLOCK_TICKS_MAX); // time since last clock is new tempo
        tempo_setting = tempo = 1000000 / ticks_per_beat; // imprecise, for display purposes
    }

    int8_t GetMultiply(int ch = 0) {return tocks_per_beat[ch];}
    int GetClockPPQN() { return clock_ppqn; }

    void SetShuffle(int8_t sh_) { shuffle_setting = shuffle = constrain(sh_, 0, 99); }
    int8_t GetShuffle() { return shuffle_setting; }

    /* Gets the current tempo. This can be used between client processes, like two different
     * hemispheres.
     */
    int GetTempo() {return tempo_setting;}
    float GetTempoFloat() {
      return 1000000.0f / ticks_per_beat;
    }
    uint32_t GetTempoTicks() {return ticks_per_beat;}
    uint32_t GetCycleTicks(int ch = 0) {
      if (tocks_per_beat[ch] > 0) return ticks_per_beat / tocks_per_beat[ch];
      if (tocks_per_beat[ch] < 0) return ticks_per_beat * (1 - tocks_per_beat[ch]);
      return 0;
    }

    void BeatSync(std::function<void()> func) {
      // TODO: prevent duplicates...
      syncfn_queue.emplace(func);
    }
    void ProcessBeatSync() {
      if (syncfn_queue.empty()) return;
      // Things that should only happen on the downbeat
      // such as: preset load, multiplier change, etc...
      while (!syncfn_queue.empty()) {
        syncfn_queue.front()();
        syncfn_queue.pop();
      }
    }

    const uint32_t BeatTick() const {
      return beat_tick; // + beat_count * ticks_per_beat;
    }
    // Reset - Resync multipliers, optionally skipping the first tock
    void Reset(bool count_skip = 0) {
      ++beat_count;
      beat_tick = micros() / 60; // HS::get_tick();
      if (!count_skip) {
        beat_count = 0;
        clock_tick[0] = 0;
        clock_tick[1] = 0;
        cycle = 1;
      }

      for (int ch = 0; ch < NR_OF_CLOCKS; ch++) {
        if (tocks_per_beat[ch] > 0 || !count_skip) count[ch] = count_skip;
      }

      cycle = 1 - cycle;
    }

    // Nudge - Used to align the internal clock with incoming clock pulses
    // The rationale is that it's better to be short by 1 than to overshoot by 1
    void Nudge(int diff) {
        if (diff > 0) diff--;
        if (diff < 0) diff++;
        beat_tick += diff; // hmmmm
    }

    // call this on every tick when clock is running, before all Controllers
    void SyncTrig(bool clocked, bool midi_sync = false);

    void Start(bool p = 0) {
        Reset();
        running = 1;
        paused = p;
        auto_reset = !p;
        if (!p && midi_out_enabled) {
            // TODO: DeferTask?
#ifdef ARDUINO_TEENSY41
            if (~midi_clktx_disable & mMaskUSBDev)
              usbMIDI.sendRealTime(usbMIDI.Start);
            if (~midi_clktx_disable & mMaskUSBHost)
              usbHostMIDI[0].sendRealTime(usbMIDI.Start);
            if (~midi_clktx_disable & mMaskUSBHost2)
              usbHostMIDI[1].sendRealTime(usbMIDI.Start);
            if (~midi_clktx_disable & mMaskSerial)
              MIDI1.sendRealTime(midi::MidiType(usbMIDI.Start));
#else
            usbMIDI.sendRealTime(usbMIDI.Start);
#endif
        }
    }

    void Stop() {
        running = 0;
        paused = 0;
        extsync = false;
        if (midi_out_enabled) {
#ifdef ARDUINO_TEENSY41
            // TODO: DeferTask?
            if (~midi_clktx_disable & mMaskUSBDev)
              usbMIDI.sendRealTime(usbMIDI.Stop);
            if (~midi_clktx_disable & mMaskUSBHost)
              usbHostMIDI[0].sendRealTime(usbMIDI.Stop);
            if (~midi_clktx_disable & mMaskUSBHost2)
              usbHostMIDI[1].sendRealTime(usbMIDI.Stop);
            if (~midi_clktx_disable & mMaskSerial)
              MIDI1.sendRealTime(midi::MidiType(usbMIDI.Stop));
#else
            usbMIDI.sendRealTime(usbMIDI.Stop);
#endif
        }
        EnableMIDIOut();
    }

    void Pause() {paused = 1;}

    void Modulate(int tempo_diff, int shuffle_diff) {
      shuffle = constrain(shuffle_setting + shuffle_diff, 0, 99);
      tempo = constrain(tempo_setting + tempo_diff, CLOCK_TEMPO_MIN, CLOCK_TEMPO_MAX);
      ticks_per_beat = 1000000 / tempo;
    }

    bool IsRunning() const {return (running && !paused);}

    bool IsPaused() const {return paused;}

    // beep boop
    void Boop(int ch = 0) {
        boop[ch] = true;
    }
    bool Beep(int ch = 0) {
        if (boop[ch]) {
            boop[ch] = false;
            return true;
        }
        return false;
    }

    /* Returns true if the clock should fire on this tick, based on the current tempo and multiplier */
    bool Tock(int ch = 0) const {
        return tock[ch];
    }

    // Returns true if MIDI Clock should be sent on this tick
    bool MIDITock() const {
        return midi_out_enabled && Tock(MIDI_CLOCK);
    }

    bool EndOfBeat(int ch = 0) const {
      return Tock(ONE_BEAT_CLOCK);
    }

    bool Cycle(int ch = 0) {return cycle;}
};

extern ClockManager clock_m;

}

#endif // CLOCK_MANAGER_H
