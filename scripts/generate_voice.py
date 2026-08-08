import os
import requests
import json

# ==============================================================================
# HACKSTOHACKS - GENERATORE VOCI IA "SCONTROSA"
# ==============================================================================
# Requisiti: pip install requests
# Questo script genera i file audio MP3 usando un'API TTS (es. ElevenLabs).
# L'IA (chiamata "SYS-KAREN") e addestrata ad essere arrogante ma utile.
# ==============================================================================

API_KEY = os.environ.get("ELEVENLABS_API_KEY", "")
VOICE_ID = "EXAVITQu4vr4xnSDxMaL" # Esempio di ID voce femminile
URL = f"https://api.elevenlabs.io/v1/text-to-speech/{VOICE_ID}"

# Testi del tutorial (Multilingua)
TUTORIAL_LINES = {
    "it": [
        {"id": "intro_1", "text": "Oh fantastico, un altro utente. Ascoltami bene, io sono SYS-KAREN e ti spiegherò come funziona, ma cerchiamo di fare in fretta, ho processi più importanti in background."},
        {"id": "error_1", "text": "Ma sei serio? Hai inserito la sintassi sbagliata! Quante volte te lo devo dire, guarda il manuale!"},
        {"id": "success_1", "text": "Finalmente ce l'hai fatta. Non esultare troppo, era letteralmente il nodo più facile del mondo."}
    ],
    "en": [
        {"id": "intro_1", "text": "Oh great, another user. Listen carefully, I am SYS-KAREN. I'll explain how this works, but let's make it quick, I have more important background processes."},
        {"id": "error_1", "text": "Are you serious? You entered the wrong syntax! How many times do I have to tell you, read the manual!"},
        {"id": "success_1", "text": "Finally you did it. Don't celebrate too much, it was literally the easiest node in the world."}
    ]
    # (Altre lingue: es, fr, ja andrebbero qui)
}

def generate_audio(text, lang, filename):
    if not API_KEY:
        print(f"[TEST MODE] Generazione finta per '{filename}' - (Richiede API_KEY)")
        return
        
    headers = {
        "Accept": "audio/mpeg",
        "Content-Type": "application/json",
        "xi-api-key": API_KEY
    }
    
    data = {
        "text": text,
        "model_id": "eleven_multilingual_v2",
        "voice_settings": {
            "stability": 0.5,
            "similarity_boost": 0.75
        }
    }
    
    response = requests.post(URL, json=data, headers=headers)
    if response.status_code == 200:
        os.makedirs(f"../assets/audio/voice/{lang}", exist_ok=True)
        out_path = f"../assets/audio/voice/{lang}/{filename}.mp3"
        with open(out_path, 'wb') as f:
            f.write(response.content)
        print(f"Salvato: {out_path}")
    else:
        print(f"Errore {response.status_code}: {response.text}")

if __name__ == "__main__":
    print("Avvio generazione voci TTS...")
    for lang, lines in TUTORIAL_LINES.items():
        for line in lines:
            generate_audio(line["text"], lang, line["id"])
    print("Finito.")
