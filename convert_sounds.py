#!/usr/bin/env python3
import os
import sys
import subprocess
import tempfile

def convert_mp3_to_header(mp3_file, output_dir):
    """Convert MP3 file to 22kHz 8-bit PCM and generate C header."""
    base_name = os.path.splitext(os.path.basename(mp3_file))[0]
    header_name = f"sound_{base_name}.h"
    header_path = os.path.join(output_dir, header_name)
    
    # Create temporary PCM file
    with tempfile.NamedTemporaryFile(suffix='.pcm', delete=False) as temp_pcm:
        temp_pcm_path = temp_pcm.name
    
    try:
        # Convert MP3 to 22kHz 8-bit unsigned PCM using ffmpeg
        cmd = [
            'ffmpeg', '-i', mp3_file,
            '-ar', '22050',      # 22kHz sample rate
            '-ac', '1',          # mono
            '-af', 'volume=2',   # 3x volume boost (adjust as needed)
            '-f', 's16le',       # 16-bit unsigned PCM
            '-y',                # overwrite output
            temp_pcm_path
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Error converting {mp3_file}: {result.stderr}")
            return False
        
        # Read PCM data and convert to PWM values
        with open(temp_pcm_path, 'rb') as f:
            pcm_data = f.read()
        
        # Convert 16-bit signed PCM to PWM values (0-2838)
        import struct
        samples = struct.unpack('<' + 'h' * (len(pcm_data) // 2), pcm_data)
        pwm_data = []
        for sample in samples:
            # Convert signed 16-bit (-32768 to +32767) to PWM range (0 to 1088)
            unsigned_sample = sample + 32768  # Make unsigned (0 to 65535)
            pwm_value = (unsigned_sample * 1088) // 65535  # Scale to PWM range
            pwm_data.append(pwm_value)
        
        # Generate C header
        array_name = f"sound_{base_name}_data"
        size_name = f"sound_{base_name}_size"
        
        with open(header_path, 'w') as f:
            f.write(f"// Auto-generated from {os.path.basename(mp3_file)}\n")
            f.write(f"// 22kHz PWM values (0-1088)\n\n")
            f.write(f"#ifndef SOUND_{base_name.upper()}_H\n")
            f.write(f"#define SOUND_{base_name.upper()}_H\n\n")
            f.write(f"#include <stddef.h>\n\n")
            f.write(f"extern const unsigned short {array_name}[];\n")
            f.write(f"extern const size_t {size_name};\n\n")
            f.write(f"const unsigned short {array_name}[] = {{\n")
            
            # Write PWM data as hex values, 8 per line
            for i in range(0, len(pwm_data), 8):
                chunk = pwm_data[i:i+8]
                hex_values = ', '.join(f'0x{v:04x}' for v in chunk)
                f.write(f"    {hex_values}")
                if i + 8 < len(pwm_data):
                    f.write(',')
                f.write('\n')
            
            f.write(f"}};\n\n")
            f.write(f"const size_t {size_name} = {len(pwm_data)};\n\n")
            f.write(f"#endif // SOUND_{base_name.upper()}_H\n")
        
        print(f"Generated {header_name} ({len(pcm_data)} bytes)")
        return True
        
    finally:
        # Clean up temporary file
        if os.path.exists(temp_pcm_path):
            os.unlink(temp_pcm_path)

def main():
    if len(sys.argv) != 3:
        print("Usage: convert_sounds.py <sounds_dir> <output_dir>")
        sys.exit(1)
    
    sounds_dir = sys.argv[1]
    output_dir = sys.argv[2]
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Convert all MP3 files
    success_count = 0
    total_count = 0
    
    for filename in sorted(os.listdir(sounds_dir)):
        if filename.lower().endswith('.mp3'):
            total_count += 1
            mp3_path = os.path.join(sounds_dir, filename)
            if convert_mp3_to_header(mp3_path, output_dir):
                success_count += 1
    
    print(f"Converted {success_count}/{total_count} sound files")
    
    # Generate index header with all sounds
    index_header = os.path.join(output_dir, "sounds.h")
    with open(index_header, 'w') as f:
        f.write("// Auto-generated sound files index\n\n")
        f.write("#ifndef SOUNDS_H\n")
        f.write("#define SOUNDS_H\n\n")
        
        # Include all individual headers
        for i in range(1, 13):  # 01.mp3 to 12.mp3
            f.write(f'#include "sound_{i:02d}.h"\n')
        
        f.write("\n#endif // SOUNDS_H\n")
    
    print(f"Generated sounds.h index header")

if __name__ == "__main__":
    main()