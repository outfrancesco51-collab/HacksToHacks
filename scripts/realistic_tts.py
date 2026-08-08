import os
import json
import asyncio
try:
    import edge_tts
except ImportError:
    print("Errore: la libreria 'edge-tts' non e' installata. Esegui: pip install edge-tts")
    exit(1)

# Impostazioni voce
# Voci disponibili (IT): it-IT-ElsaNeural (Female), it-IT-IsabellaNeural (Female)
# Voci disponibili (EN): en-US-AriaNeural (Female), en-US-SteffanNeural (Male)
VOICE_IT = "it-IT-ElsaNeural"
VOICE_EN = "en-US-AriaNeural"

# Percorsi
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOCALES_DIR = os.path.join(SCRIPT_DIR, "..", "locales")
AUDIO_DIR = os.path.join(SCRIPT_DIR, "..", "assets", "audio", "voice")

async def generate_audio(text, voice, output_path, rate="+10%", pitch="-5Hz"):
    print(f"Generazione [{voice}]: {output_path}")
    communicate = edge_tts.Communicate(text, voice, rate=rate, pitch=pitch)
    await communicate.save(output_path)

async def main():
    keys_to_generate = ["TUTORIAL_SYSKAREN_1", "SYSKAREN_ERROR", "SYSKAREN_SUCCESS"]
    
    # IT
    it_path = os.path.join(LOCALES_DIR, "it.json")
    if os.path.exists(it_path):
        with open(it_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
            os.makedirs(os.path.join(AUDIO_DIR, "it"), exist_ok=True)
            for k in keys_to_generate:
                if k in data:
                    out_path = os.path.join(AUDIO_DIR, "it", f"{k.lower()}.wav")
                    # Sweet voice modulation for SUCCESS
                    if k == "SYSKAREN_SUCCESS":
                        await generate_audio(data[k], VOICE_IT, out_path, rate="-15%", pitch="+10Hz")
                    else:
                        await generate_audio(data[k], VOICE_IT, out_path, rate="+10%", pitch="-5Hz")
                    
    # EN
    en_path = os.path.join(LOCALES_DIR, "en.json")
    if os.path.exists(en_path):
        with open(en_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
            os.makedirs(os.path.join(AUDIO_DIR, "en"), exist_ok=True)
            for k in keys_to_generate:
                if k in data:
                    out_path = os.path.join(AUDIO_DIR, "en", f"{k.lower()}.wav")
                    if k == "SYSKAREN_SUCCESS":
                        await generate_audio(data[k], VOICE_EN, out_path, rate="-15%", pitch="+10Hz")
                    else:
                        await generate_audio(data[k], VOICE_EN, out_path, rate="+10%", pitch="-5Hz")

if __name__ == "__main__":
    asyncio.run(main())
