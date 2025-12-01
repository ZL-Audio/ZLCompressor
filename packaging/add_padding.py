import sys
import os
from PIL import Image

def add_macos_padding(input_path, output_path=None, target_size=824, vertical_shift=0):
    """
    Resizes an image to fit the macOS icon grid (approx 824px) 
    and centers it on a 1024x1024 transparent canvas.
    
    Args:
        input_path (str): Path to the source image.
        output_path (str): Path to save the result. If None, appends '_padded' to filename.
        target_size (int): The size of the inner icon (default 824 for macOS standard).
        vertical_shift (int): Pixels to shift the icon UP (optical centering). 
                              Use ~20 for optical centering if adding a shadow later.
    """
    
    CANVAS_DIMENSION = 1024

    if not os.path.exists(input_path):
        print(f"Error: File not found at {input_path}")
        return

    try:
        # 1. Open the original image
        original = Image.open(input_path).convert("RGBA")
        
        # 2. Create the blank 1024x1024 transparent canvas
        canvas = Image.new("RGBA", (CANVAS_DIMENSION, CANVAS_DIMENSION), (0, 0, 0, 0))
        
        # 3. Calculate aspect-ratio safe resizing
        # We use .thumbnail() logic to ensure we don't distort non-square images,
        # though standard icons should usually be square.
        width_ratio = target_size / original.width
        height_ratio = target_size / original.height
        scale_factor = min(width_ratio, height_ratio)
        
        new_width = int(original.width * scale_factor)
        new_height = int(original.height * scale_factor)
        
        # High-quality resampling (LANCZOS) is crucial for icons
        resized_icon = original.resize((new_width, new_height), Image.Resampling.LANCZOS)
        
        # 4. Calculate position to center the image
        # Mathematical center
        x_pos = (CANVAS_DIMENSION - new_width) // 2
        y_pos = (CANVAS_DIMENSION - new_height) // 2
        
        # Apply optional vertical shift (subtracting moves it UP)
        y_pos -= vertical_shift

        # 5. Paste the resized icon onto the canvas
        # We use resized_icon as the mask to preserve transparency
        canvas.paste(resized_icon, (x_pos, y_pos), resized_icon)
        
        # 6. Save the output
        if output_path is None:
            filename, ext = os.path.splitext(input_path)
            output_path = f"{filename}_padded{ext}"
            
        canvas.save(output_path, "PNG")
        print(f"Success! Saved padded icon to: {output_path}")
        print(f"Dimensions: {CANVAS_DIMENSION}x{CANVAS_DIMENSION} (Content: {new_width}x{new_height})")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python add_padding.py <path_to_image.png> [optional_vertical_shift]")
        print("Example: python add_padding.py my_icon.png")
    else:
        input_file = sys.argv[1]
        
        # Optional: Allow user to pass a vertical shift as 2nd argument
        v_shift = int(sys.argv[2]) if len(sys.argv) > 2 else 0
        
        add_macos_padding(input_file, vertical_shift=v_shift)