import os
from PIL import Image

def raw_fnt_to_png(fnt_path, png_path, char_width=8):
    file_size = os.path.getsize(fnt_path)
    
    if file_size % 256 != 0:
        print("Warning: File size is not a multiple of 256. This may not be a raw binary font.")
    
    char_height = file_size // 256
    
    img_height = 256 * char_height
    
    img = Image.new('L', (char_width, img_height), 0)
    
    with open(fnt_path, 'rb') as f:
        data = f.read()
        
    for char_index in range(256):
        y_offset = char_index * char_height
        
        for y in range(char_height):
            byte_index = (char_index * char_height) + y
            
            if byte_index >= len(data):
                break
                
            row_byte = data[byte_index]
            
            for x in range(char_width):
                if (row_byte >> (7 - x)) & 1:
                    img.putpixel((x, y_offset + y), 255)
                    
    img.save(png_path)
    print(f"Generated font map: {png_path} ({char_width}x{img_height} pixels)")

if __name__ == "__main__":
    raw_fnt_to_png("font.fnt", "font.png")
