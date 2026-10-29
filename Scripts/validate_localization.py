"""Check all current runtime copy has complete translations and bundled fonts."""
from pathlib import Path
import json
import re

root = Path(__file__).resolve().parents[1]
data = json.loads((root / "DesignData/localization.json").read_text())
assert data["default"] == "en", "New runs must default to English"
assert set(data["languages"]) == {"en", "zh", "ja", "ko", "es", "fr", "de"}
optional = set(data["languages"]) - {"en"}
strings = data["strings"]
for english, translations in strings.items():
    assert english.strip(), "Empty source string"
    assert set(translations) == optional, f"Missing language: {english}"
    assert all(value.strip() for value in translations.values()), f"Empty translation: {english}"

required = set()
for path in (root / "Source/LastMinuteCourier").glob("*.cpp"):
    source = path.read_text()
    required.update(re.findall(r'(?:Copy|Notify)\(\s*TEXT\("((?:[^"\\]|\\.)*)"\),', source))
    tutorial = re.search(r'TutorialText\s*=\s*\{(.*?)\};', source, re.S)
    if tutorial:
        required.update(re.findall(r'TEXT\("((?:[^"\\]|\\.)*)"\)', tutorial.group(1)))
content = json.loads((root / "DesignData/game_content.json").read_text())
for card in content["opening_cards"]:
    required.add(card["title"]["en"])
    required.add(card["body"]["en"])
assert not required - strings.keys(), f"Untranslated runtime copy: {required - strings.keys()}"

for name in ("AtkinsonHyperlegible-Regular.ttf", "BreeSerif-Regular.ttf", "NotoSansSC-Regular.otf",
             "NotoSansJP-Regular.otf", "NotoSansKR-Regular.otf"):
    with (root / "Content/Fonts" / name).open("rb") as font:
        assert font.read(4) in (b"OTTO", b"\x00\x01\x00\x00"), f"Invalid font: {name}"
for name in ("Atkinson-OFL.txt", "BreeSerif-OFL.txt", "NotoSansSC-OFL.txt"):
    assert b"SIL OPEN FONT LICENSE" in (root / "Content/Fonts" / name).read_bytes(), name

print(f"PASS: {len(required)} runtime strings; {len(optional)} optional languages; fonts and licenses present.")
print("This checks coverage and files. Native-language review and visual playtesting remain separate.")
