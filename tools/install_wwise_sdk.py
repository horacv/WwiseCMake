"""
Wwise SDK Installer for CI/CD
Copies Wwise SDK files to the project.
"""

import sys
import shutil
from pathlib import Path

# Project directories
PROJECT_ROOT = Path(__file__).parent.parent
WWISE_LIB_DIR = PROJECT_ROOT / "libs" / "wwise"

# Platform-specific configuration
PLATFORM_CONFIG = {
    'windows': {
        'lib_subdir': 'x64_vc170',
        },
    'mac': {
        'lib_subdir': 'Mac_Xcode2600',
    }
}

def copy_sdk_files(source_dir, platform):
    """Copy Wwise SDK files from source directory to project."""
    print("\nCopying Wwise SDK files to project...")

    config = PLATFORM_CONFIG[platform]
    lib_subdir = config['lib_subdir']

    # Find the actual SDK directory in temp (may be nested)
    sdk_dirs = list(source_dir.rglob('include'))
    if not sdk_dirs:
        sys.exit(1)

    source_root = sdk_dirs[0].parent if sdk_dirs else source_dir

    # Copy core headers
    src = source_root / "include"
    dst = WWISE_LIB_DIR / "include"
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied SDK headers ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: SDK headers not found at {src}")

    # Copy libraries
    src = source_root / lib_subdir
    dst = WWISE_LIB_DIR / "lib" / lib_subdir
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied libraries ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: SDK libraries not found at {src}")

    # Copy samples/SoundEngine (Wwise Streaming Manager)
    src = source_root / "samples" / "SoundEngine"
    dst = WWISE_LIB_DIR / "samples" / "SoundEngine"
    if src.exists():
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(src, dst, dirs_exist_ok=True)
        print(f"✓ Copied samples/SoundEngine source ({len(list(dst.glob('*')))} files)")
    else:
        print(f"⚠ Warning: samples/SoundEngine not found at {src}")


def main():
    if len(sys.argv) < 3:
        print("Usage: python install_wwise_sdk.py <sdk_path> <platform>")
        print("Example: python install_wwise_sdk.py C:\Audiokinetic\Wwise2024.1.12.9034\SDK windows")
        sys.exit(1)

    sdk_path = Path(sys.argv[1])
    platform = sys.argv[2]

    # Validate installer exists
    if not sdk_path.exists():
        print(f"Error: SDK folder not found: {sdk_path}")
        sys.exit(1)

    print("\n" + "=" * 60)
    print(f"Wwise SDK Installer")
    print("\n" + "=" * 60)

    try:
        # Copy SDK files to the project
        copy_sdk_files(sdk_path, platform)

        print("\n✓ Installation complete!")
        print(f"Wwise SDK files copied to: {WWISE_LIB_DIR}")

    except Exception as e:
        print(f"\nError: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()