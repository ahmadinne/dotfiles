import re

# Your Paradise color mapping (Old Everforest Hex : New Paradise Hex)
replacements = {
    '1e2326': '050505',
    '272e33': '070707',
    '5c6a72': 'e8e3e3',
    'f85552': 'b66467',
    'dfa000': 'd9bc8c',
    '8da101': '8c977d',
    '3a94c5': '8da3b9',
    'df69ba': 'a988b0',
    '35a77c': '8aa6a2',
    'fffbef': 'e8e3e3',
    '7fbbb3': '8da3b9',
    '293136': '070707',
    '232a2e': '050505'
}

def hex_to_rgb(hex_val):
    """Converts a hex string to an (R, G, B) tuple."""
    return tuple(int(hex_val[i:i+2], 16) for i in (0, 2, 4))

# Read the original CSS file
try:
    with open('everforest.css', 'r') as f:
        content = f.read()
except FileNotFoundError:
    print("Error: Could not find 'everforest.css'. Make sure it's in the same folder.")
    exit(1)

for old_hex, new_hex in replacements.items():
    # 1. Replace Hex colors (case-insensitive)
    hex_pattern = re.compile(re.escape(f'#{old_hex}'), re.IGNORECASE)
    content = hex_pattern.sub(f'#{new_hex}', content)
    
    # 2. Convert to RGB to find and replace rgba(...) values
    old_rgb = hex_to_rgb(old_hex)
    new_rgb = hex_to_rgb(new_hex)
    
    # Pattern to match rgba(R, G, B, alpha) while keeping the alpha intact
    old_rgba_pattern = re.compile(rf'rgba\(\s*{old_rgb[0]}\s*,\s*{old_rgb[1]}\s*,\s*{old_rgb[2]}\s*,', re.IGNORECASE)
    new_rgba_replacement = f'rgba({new_rgb[0]}, {new_rgb[1]}, {new_rgb[2]},'
    
    content = old_rgba_pattern.sub(new_rgba_replacement, content)

# Save the new Paradise CSS file
with open('paradise.css', 'w') as f:
    f.write(content)

print("Success! Your Paradise theme has been saved to 'paradise.css'.")
