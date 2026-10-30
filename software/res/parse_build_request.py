import os

comment_text = os.environ['GH_COMMENT']

# Accepts plain flags or "oc-build/1 target=... apps=+A,+B" requests.
flags = []
for item in comment_text.replace(',', ' ').replace(';', ' ').split():
    if '=' in item:
        key, item = item.split('=', 1)
        if key.lower() == 'target':
            continue
    flags.append(item.lstrip('+'))

custom_defines = "-DCUSTOM_BUILD"
pewcount = 0

for item in flags:
    f = item.strip().upper()
    if f.startswith('NOHEMI') or f.startswith('NO_HEM'):
        custom_defines += " -DNO_HEMISPHERE"
    if f.startswith('NOWAVE') or f.startswith('NO_WAVE'):
        custom_defines += " -DNO_WAVEFORM_EDITOR"
    if f.startswith('NOCVSEQ') or f.startswith('NO_CVSEQ'):
        custom_defines += " -DNO_APPLET_CVSEQ"
    if f == 'LTO':
        custom_defines += " -DTEENSY_OPT_SMALLEST_CODE_LTO"
    if f.startswith('VOR'):
        custom_defines += " -DVOR"
    if f.startswith('BUCHLA') or f.startswith('NLM') or f.startswith('NORTHERN'):
        custom_defines += " -DNORTHERNLIGHT"
    if f.startswith('NLM_HOC'):
        custom_defines += " -DNLM_hOC"
    if f.startswith('NLM_CARDOC'):
        custom_defines += " -DNLM_cardOC"
    if f.startswith('NLM_2OC_L'):
        custom_defines += " -DNORTHERNLIGHT_2OC_LEFTSIDE"
    if f.startswith('CALIBR'):
        custom_defines += " -DENABLE_APP_CALIBR8OR"
    if f.startswith('SCENE'):
        custom_defines += " -DENABLE_APP_SCENES"
    if f.startswith('PONG'):
        custom_defines += " -DENABLE_APP_PONG"
    if f.startswith('ENIGMA'):
        custom_defines += " -DENABLE_APP_ENIGMA"
    if f.startswith('CAPTAIN') or f.startswith('MIDI'):
        custom_defines += " -DENABLE_APP_MIDI"
    if f.startswith('PIQUED'):
        custom_defines += " -DENABLE_APP_PIQUED"
    if f.startswith('QUADRAT') or f.startswith('POLYLFO'):
        custom_defines += " -DENABLE_APP_POLYLFO"
    if f.startswith('HARRING') or f.startswith('H1200'):
        custom_defines += " -DENABLE_APP_H1200"
    if f.startswith('BYTE') or f.startswith('VIZNUT'):
        custom_defines += " -DENABLE_APP_BYTEBEATGEN"
    if f.startswith('NEURAL'):
        custom_defines += " -DENABLE_APP_NEURAL_NETWORK"
    if f.startswith('DARKEST'):
        custom_defines += " -DENABLE_APP_DARKEST_TIMELINE"
    if f.startswith('LOW-RENT') or f.startswith('LORENZ'):
        custom_defines += " -DENABLE_APP_LORENZ"
    if f.startswith('COPIER') or f.startswith('ASR'):
        custom_defines += " -DENABLE_APP_ASR"
    if f.startswith('QUANTER'):
        custom_defines += " -DENABLE_APP_QUANTERMAIN"
    if f.startswith('META'):
        custom_defines += " -DENABLE_APP_METAQ"
    if f.startswith('ACID') or f.startswith('CHORDS'):
        custom_defines += " -DENABLE_APP_CHORDS"
    if f.startswith('PASSEN'):
        custom_defines += " -DENABLE_APP_PASSENCORE"
    if f.startswith('SEQUIN'):
        custom_defines += " -DENABLE_APP_SEQUINS"
    if f.startswith('AUTOMATON'):
        custom_defines += " -DENABLE_APP_AUTOMATONNETZ"
    if f.startswith('DIALECT') or f.startswith('BBGEN'):
        custom_defines += " -DENABLE_APP_BBGEN"
    if f.startswith('REFS') or f.startswith('REFERENCE'):
        custom_defines += " -DENABLE_APP_REFERENCES"
    if f.startswith('GRIDS2'):
        custom_defines += " -DDRUMMAP_GRIDS2"
    if f == 'LEANPLUS':
        custom_defines += " -DPHZ_LEAN_EXTRAS"
    if f.startswith('MOAR_PRESETS'):
        custom_defines += " -DMOAR_PRESETS"

    # applet categories
    if f.startswith('APPLET_SEQ'):
        custom_defines += " -DAPPLETS_SEQUENCING"
    if f.startswith('APPLET_Q'):
        custom_defines += " -DAPPLETS_QUANTIZERS"
    if f.startswith('APPLET_MOD'):
        custom_defines += " -DAPPLETS_MODULATION"
    if f.startswith('APPLET_UTIL'):
        custom_defines += " -DAPPLETS_LOGIC_UTILITY"
    if f.startswith('APPLET_WEIRD') or f.startswith('WEIRD'):
        custom_defines += " -DAPPLETS_WEIRDSTUFF"
    if f.startswith('APPLET_CRAZY') or f.startswith('CRAZY'):
        custom_defines += " -DAPPLETS_CRAZYTOWN"

    if f.startswith('PEWPEWPEW'):
        custom_defines += " -DPEWPEWPEW"
    elif f.startswith('PEW'):
        pewcount += 1

    #TODO proper lookup table

if pewcount > 2:
    custom_defines += " -DPEWPEWPEW"

print(custom_defines)
os.environ["CUSTOM_BUILD_FLAGS"] = custom_defines
