param (
    [string]$Lang = "it"
)

$OutputFolder = Join-Path $PSScriptRoot "..\assets\audio\voice"

# Assicurati che la cartella di output esista
$TargetFolder = Join-Path $OutputFolder $Lang
if (-Not (Test-Path $TargetFolder)) {
    New-Item -ItemType Directory -Force -Path $TargetFolder | Out-Null
}

Add-Type -AssemblyName System.Speech
$synth = New-Object System.Speech.Synthesis.SpeechSynthesizer

# Scegli una voce in base alla lingua (se disponibile)
$voices = $synth.GetInstalledVoices()
$voiceName = ""

foreach ($v in $voices) {
    if ($v.VoiceInfo.Culture.Name.StartsWith($Lang)) {
        if ($v.VoiceInfo.Gender -eq 'Female') {
            $voiceName = $v.VoiceInfo.Name
            break
        }
    }
}

if ($voiceName -ne "") {
    $synth.SelectVoice($voiceName)
    Write-Host "Voce selezionata: $voiceName"
} else {
    Write-Host "Nessuna voce femminile trovata per la lingua '$Lang'. Uso la predefinita."
}

# Imposta effetto "Scontroso/Robotico"
$synth.Rate = 2
$synth.Volume = 100

# Dizionario testi
$texts = @{
    "intro_1" = "Oh fantastico, un altro utente. Ascoltami bene, io sono SYS-KAREN. Sbrigati, ho processi più importanti in background."
    "error_1" = "Ma sei serio? Hai inserito la sintassi sbagliata! Quante volte te lo devo dire, guarda il manuale!"
    "success_1" = "Finalmente ce l'hai fatta. Non esultare troppo, era letteralmente il nodo più facile del mondo."
}

if ($Lang -eq "en") {
    $texts = @{
        "intro_1" = "Oh great, another user. Listen carefully, I am SYS-KAREN. Hurry up, I have more important background processes."
        "error_1" = "Are you serious? You entered the wrong syntax! How many times do I have to tell you, read the manual!"
        "success_1" = "Finally you did it. Don't celebrate too much, it was literally the easiest node in the world."
    }
}

foreach ($key in $texts.Keys) {
    $outFile = Join-Path $TargetFolder "$key.wav"
    $synth.SetOutputToWaveFile($outFile)
    $synth.Speak($texts[$key])
    Write-Host "Generato: $outFile"
}

$synth.SetOutputToDefaultAudioDevice()
$synth.Dispose()
Write-Host "Generazione completata."
