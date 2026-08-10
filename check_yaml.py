import yaml
import sys

def check(file):
    try:
        with open(file, 'r', encoding='utf-8') as f:
            yaml.safe_load(f)
            print(f"{file} is valid YAML")
    except Exception as e:
        print(f"Error in {file}: {e}")

check('.github/workflows/ios.yml')
check('.github/workflows/android.yml')
